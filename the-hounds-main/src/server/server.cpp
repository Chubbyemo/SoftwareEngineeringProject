#include "server/server.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "shared/game.hpp"
#include "shared/messages.hpp"

// ID Assignment order for new connections
const std::vector<int> Server::idAssignmentOrder{0, 2, 1, 3};

// Constructor: Initializes the server with the given address, port, and
// connection timeout limit
Server::Server(std::string serverAddress, int port, int connectionTimeout)
    : serverAddress_(std::move(serverAddress)), port_(port), acceptor_() {
  if (!acceptor_.open(sockpp::inet_address(serverAddress_, port_))) {
    throw std::runtime_error("Error creating the server: " +
                             acceptor_.last_error_str());
  }

  // Initialize players (inactive)
  for (int i = 0; i < 4; ++i) {
    players_[i].id = i;
  }
}

// Destructor
Server::~Server() {
  stop();  // Ensure all connections are properly closed
}

// Starts the server and waits for players before running the game loop
void Server::start() {
  if (!acceptor_.is_open()) {
    throw std::runtime_error("Error starting server: acceptor not running.");
  }

  running_ = true;

  log("Server listening on " + serverAddress_ + ":" + std::to_string(port_) +
      ", waiting for players...");

  waitForPlayers();
  stop();
}

void Server::stop() {
  static std::atomic<bool> stopped{false};
  if (stopped.exchange(true)) {
    return;
  }

  log("Shutting down server");
  shuttingDown_ = true;

  if (acceptor_.is_open()) {
    acceptor_.shutdown();
    acceptor_.close();
  }

  // Close sockets
  {
    std::lock_guard<std::mutex> lock(playersMutex_);
    for (auto& player : players_) {
      if (player.socket) {
        player.socket->shutdown();
      }
    }
  }

  // Clean up threads
  {
    std::lock_guard<std::mutex> lock(threadsMutex_);
    auto self = std::this_thread::get_id();

    for (auto& t : clientThreads_) {
      if (t.joinable() && t.get_id() != self) {
        t.join();
      }
    }
    clientThreads_.clear();
  }

  // Clean up players
  {
    std::lock_guard<std::mutex> lock(playersMutex_);
    for (auto& player : players_) {
      if (player.socket) {
        player.socket->close();
        player.socket.reset();
      }
      player.isActive = false;
      player.isReady = false;
    }
  }

  log("Server stopped.");
}

std::array<std::optional<std::string>, 4> Server::getPlayerNames() const {
  std::lock_guard<std::mutex> lock(playersMutex_);

  std::array<std::optional<std::string>, 4> names;
  for (int i = 0; i < 4; i++) {
    if (players_[i].isActive) {
      names[i] = players_[i].name;
    }
  }
  return names;
}

int Server::getNumPlayers() const {
  std::lock_guard<std::mutex> lock(playersMutex_);
  return numPlayers_;
}

void Server::waitForPlayers() {
  while (running_) {
    log("Waiting for players to connect");

    sockpp::tcp_socket sock = acceptor_.accept();
    if (!sock) {
      // Check if server is running
      if (!running_) {
        return;
      }

      throw std::runtime_error("Error accepting connection: " +
                               acceptor_.last_error_str());
    }

    // Handle the new connection
    int id = handleNewConnection(std::move(sock));
    if (id < 0) {
      continue;  // Game is full, connection rejected.
    }

    // Start a thread to process actions for this client
    {
      std::lock_guard<std::mutex> lock(threadsMutex_);
      clientThreads_.emplace_back(&Server::handleNewMessage, this, id);
    }
    // std::thread(&Server::handleNewMessage, this, playerId).detach();

    broadcastPlayerList();
  }
}

int Server::handleNewConnection(sockpp::tcp_socket sock) {
  log("New connection request received");

  if (!running_) {
    log("Cannot accept new connections: server is not running");
    return -1;
  }

  int clientId = -1;

  {
    std::lock_guard<std::mutex> lock(playersMutex_);

    // Search for first non-occupied ID
    for (int id : idAssignmentOrder) {
      if (!players_[id].isActive) {
        clientId = id;
        break;
      }
    }
    if (clientId == -1) {
      sock.shutdown();
      sock.close();
      log("Connection error: maximum players reached.");
      return -1;
    }

    log("Connecting client with client ID " + std::to_string(clientId));

    ClientInfo& p = players_[clientId];
    p.threadId = clientId;
    p.id = clientId;
    p.socket = std::make_unique<sockpp::tcp_socket>(std::move(sock));
    p.isActive = true;  // Claim the slot immediately under lock
    p.isReady = false;
    if (p.name.empty()) {
      p.name =
          "Player " + std::to_string(clientId);  // Default username for players
    }

    ++numPlayers_;
  }

  // Send client its ID
  ConnectionResponseMessage welcomeMessage(true, "", clientId);
  messagePlayer(clientId, welcomeMessage.toJson());

  log("Player " + std::to_string(clientId) + " connected!");

  // Get player name from the client
  char buf[1024];
  ssize_t n = players_[clientId].socket->read(buf, sizeof(buf));
  if (n <= 0) {
    throw std::runtime_error("Error reading player name: " +
                             players_[clientId].socket->last_error_str());
  }

  std::string receivedData(buf, n);
  nlohmann::json receivedJson = nlohmann::json::parse(receivedData);
  auto nameMessage = Message::fromJson(receivedJson);
  auto* setNameMessage =
      static_cast<ConnectionRequestMessage*>(nameMessage.get());
  const std::string playerName = setNameMessage->name;

  if (isValidName(playerName)) {
    players_[clientId].name = playerName;
  }

  log("Player " + std::to_string(clientId) +
      " connected with name: " + players_[clientId].name);

  return clientId;
}

void Server::handlePlayCard(size_t handIndex, int playerId,
                            const PlayCardRequestMessage& req) {
  if (!gameRunning_ || !game_) {
    PlayCardResponseMessage resp(handIndex, false, "No game is running");
    return messagePlayer(playerId, resp.toJson());
  }

  auto& gs = *game_;

  // 1. Check if it's the current player's
  if (!gs.isMyTurn(static_cast<size_t>(playerId))) {
    PlayCardResponseMessage resp(handIndex, false, "Not your turn");
    return messagePlayer(playerId, resp.toJson());
  }

  try {
    // 2. Convert PlayCardRequestMessage → Move
    BraendiDog::Move target_move(req.move);

    // 3. Validate move using GameState logic
    if (!gs.isValidTurn(target_move)) {
      PlayCardResponseMessage resp(handIndex, false, "Invalid move");
      return messagePlayer(playerId, resp.toJson());
    }

    // DEGUG: cout gamestate hands
    for (size_t i = 0; i < 4; i++) {
      auto playerOpt = gs.getPlayerByIndex(i);
      if (playerOpt.has_value()) {
        const auto& player = playerOpt.value();
        const auto& hand = player.getHand();
        std::cout << "Player " << i << " hand: ";
        for (const auto& cardIdx : hand) {
          std::cout << cardIdx << " ";
        }
        std::cout << std::endl;
      }
    }

    // 4. Execute the move
    bool playerFinished = gs.executeMove(target_move);
    auto [gameEnded, roundEnded] = gs.endTurn();

    // 6. Respond
    PlayCardResponseMessage resp(handIndex, true, "");
    messagePlayer(playerId, resp.toJson());

    // 7. Broadcast updated game state
    broadcastGameState();

    // 8. If player finished, broadcast message
    if (playerFinished) {
      PlayerFinishedMessage finishMsg(static_cast<size_t>(playerId));
      broadcastMessage(finishMsg.toJson());
    }

    // 9. Broadcast game end if needed
    if (gameEnded) {
      handleGameEnd();
    }
    // 10. Deal cards again if round ended
    else if (roundEnded) {
      newRound();
    }

  } catch (const std::exception& e) {
    logError("Could not make a move — " + std::string(e.what()));

    PlayCardResponseMessage resp(handIndex, false, e.what());
    return messagePlayer(playerId, resp.toJson());
  }
}

void Server::handleSkipTurn(int playerId) {
  if (!gameRunning_ || !game_) {
    SkipTurnResponseMessage resp(false, "No game is running");
    return messagePlayer(playerId, resp.toJson());
  }

  auto& gs = *game_;

  // 1. Check if it's the current player's
  if (!gs.isMyTurn(static_cast<size_t>(playerId))) {
    SkipTurnResponseMessage resp(false, "Not your turn");
    return messagePlayer(playerId, resp.toJson());
  }

  try {
    // 2. Validate fold using GameState logic
    if (!gs.isValidTurn()) {
      SkipTurnResponseMessage resp(false, "Invalid fold - legal moves exist");
      return messagePlayer(playerId, resp.toJson());
    }

    // 3. Execute the fold
    gs.executeFold();
    auto [gameEnded, roundEnded] = gs.endTurn();

    // 4. Respond
    SkipTurnResponseMessage resp(true, "");
    messagePlayer(playerId, resp.toJson());

    // 5. Broadcast updated game state
    broadcastGameState();  // Player status has changed

    // 6. Broadcast game end if needed
    if (gameEnded) {
      handleGameEnd();
    }

    // 7. Deal cards again if round ended
    else if (roundEnded) {
      newRound();
    }

  } catch (const std::exception& e) {
    logError("Could not skip turn — " + std::string(e.what()));

    SkipTurnResponseMessage resp(false, e.what());
    return messagePlayer(playerId, resp.toJson());
  }
}

void Server::newRound() {
  log("Starting new round.");
  auto dealtCards = game_->dealCards();

  for (const auto& [id, hand] : dealtCards) {
    // Save in game state
    auto& playerOpt = game_->getPlayerByIndex(id);
    if (playerOpt.has_value()) {
      playerOpt.value().setHand(hand);
    } else {
      std::cerr << "Error: Could not find player " << id
                << " in game state when dealing new round cards!" << std::endl;
      continue;
    }
    // Send private message to each player with their dealt cards
    CardsDealtMessage cardsMsg(id, hand);
    messagePlayer(static_cast<int>(id), cardsMsg.toJson());
  }
}

void Server::handleGameEnd() {
  log("Game ended, releaseing rankings.");

  // Build a simple results message from leaderBoard
  const auto& leaderboard = game_->getLeaderBoard();

  GameResultsMessage resultsMsg(leaderboard);
  broadcastMessage(resultsMsg.toJson());

  gameRunning_ = false;
  running_ = false;

  std::this_thread::sleep_for(std::chrono::seconds(300));
  acceptor_.shutdown();
  acceptor_.close();
}

void Server::handleDisconnect(const size_t playerId) {
  {
    std::lock_guard<std::mutex> lock(playersMutex_);

    auto& p = players_[playerId];

    // Close socket
    if (p.socket) {
      p.socket->shutdown();
      p.socket->close();
    }

    // Update player info
    p.isActive = false;
    p.isReady = false;
    p.socket = nullptr;

    numPlayers_--;

    // Re-arrange Player ID's if game hasn't started
    if (!gameRunning_) {
      std::array<ClientInfo, 4> updatedPlayers;

      int assignmentIdx = 0;

      for (auto& p : players_) {
        if (p.isActive) {
          size_t updatedId = idAssignmentOrder[assignmentIdx];

          updatedPlayers[updatedId] = std::move(p);
          updatedPlayers[updatedId].id = updatedId;
          assignmentIdx++;
        }
      }
      players_ = std::move(updatedPlayers);
    }

    log("Cleaned up after disconnected player " + std::to_string(playerId));
  }

  // Send disconnect message to remaining players
  PlayerDisconnectedMessage disconnectMsg(playerId);
  broadcastMessage(disconnectMsg.toJson());

  // LOBBY -> update player list
  if (!gameRunning_) {
    broadcastPlayerList();
  }
  // MAIN GAME -> update game state
  else {
    // Update gamestate and call gamestate update
    game_->disconnectPlayer(playerId);
    broadcastGameState();

    if (!shuttingDown_ && numPlayers_ <= 1) {
      handleGameEnd();
    }
  }
}

void Server::handleNewMessage(int threadId) {
  while (true) {
    int playerId = -1;
    sockpp::tcp_socket* socket = nullptr;

    // Find current player by threadId and get socket - all under lock
    {
      std::lock_guard<std::mutex> lock(playersMutex_);

      // Find player based on thread ID
      for (auto& p : players_) {
        if (p.threadId == threadId && p.isActive) {
          playerId = p.id;
          break;
        }
      }

      // Exit if player not found or inactive
      if (playerId < 0) {
        return;
      }

      // Exit if no socket
      if (!players_[playerId].socket) {
        return;
      }

      socket = players_[playerId].socket.get();
    }

    try {
      char buf[1024];
      ssize_t n = socket->read(buf, sizeof(buf));

      // Connection closed or error reading from socket
      if (n <= 0) {
        log("Player " + std::to_string(playerId) + " disconnected.");
        handleDisconnect(playerId);
        break;
      }

      std::string message(buf, n);
      nlohmann::json messageJson = nlohmann::json::parse(message);

      log("Received message from client " + std::to_string(playerId) + ":\n " +
          message);

      auto parsedMessage = Message::fromJson(messageJson);
      MessageType messageType = parsedMessage->getMessageType();

      log("Parsed message from player " + std::to_string(playerId) + ":\n " +
          parsedMessage->toString());

      if (messageType == MessageType::REQ_READY) {
        if (game_ && gameRunning_) {
          std::string errorMsg =
              "Game is already in progress, cannot set player as ready";
          logError(errorMsg);
          ReadyResponseMessage resp = ReadyResponseMessage(false, errorMsg);
        }

        setPlayerReady(messageJson["playerId_"]);
        broadcastPlayerList();
      } else if (messageType == MessageType::REQ_START_GAME) {
        log("Player " + std::to_string(playerId) + " requested to start game");

        if (game_ && gameRunning_) {
          std::string errorMsg =
              "Game is already in progress, cannot start a new game";
          logError(errorMsg);
          StartGameResponseMessage resp =
              StartGameResponseMessage(false, errorMsg);
        }

        if (areAllPlayersReady() && numPlayers_ >= 2) {
          log("Starting game with " + std::to_string(numPlayers_));
          startGame();
        } else {
          std::string errorMsg =
              "Start Game request denied: Not all players are ready. Current "
              "number of players: " +
              std::to_string(numPlayers_);
          logError(errorMsg);
          StartGameResponseMessage resp =
              StartGameResponseMessage(false, errorMsg);
        }
      } else if (messageType == MessageType::REQ_PLAY_CARD) {
        auto* msg = static_cast<PlayCardRequestMessage*>(parsedMessage.get());
        log("Player " + std::to_string(playerId) +
            " requested to play the card at handIndex : " +
            std::to_string(msg->move.handIndex));
        handlePlayCard(msg->move.handIndex, playerId, *msg);
      } else if (messageType == MessageType::REQ_SKIP_TURN) {
        log("Player " + std::to_string(playerId) +
            " requested to skip their turn");
        handleSkipTurn(playerId);
      }
    } catch (const std::exception& ex) {
      logError("Error handling action from player " + std::to_string(playerId) +
               ": " + ex.what());
      break;
    }
  }
}

bool Server::isValidName(const std::string name) const {
  // Name must be a non-empty string
  if (name.empty()) {
    logError("Name cannot be empty");
    return false;
  }

  // Name must be unique
  for (auto& player : players_) {
    if (player.name == name) {
      logError("Player with name " + name + " already exists");
      return false;
    }
  }
  return true;
}

void Server::setPlayerReady(int playerId) {
  log("setPlayerReady Function called by " + std::to_string(playerId));

  if (playerId < 0 || playerId >= 4) {
    return;
  }

  std::lock_guard<std::mutex> lock(playersMutex_);
  auto& p = players_[playerId];
  if (!p.isActive) return;  // Check if player is active

  p.isReady = true;
  log("Player " + std::to_string(playerId) + " is ready!");
}

bool Server::areAllPlayersReady() const {
  log("areAllPlayersReady Function called.");

  std::lock_guard<std::mutex> lock(playersMutex_);
  int numReady = 0;

  for (const auto& player : players_) {
    if (!player.isActive) {
      continue;
    }

    if (!player.isReady) {
      log("Player " + std::to_string(player.id) + " is not ready");
      return false;  // If any player is not ready, return false
    }
    numReady++;
  }

  log(std::to_string(numReady) + "/" + std::to_string(numReady) +
      " players are ready.");
  return true;
}

void Server::startGame() {
  log("All players ready, starting game...");

  // Build list of players for GameState
  auto gamePlayers = getPlayerNames();

  // Initialize Game
  game_ = std::make_unique<BraendiDog::GameState>(gamePlayers);
  gameRunning_ = true;

  // Notify clients game is starting
  GameStartMessage startMsg(numPlayers_);
  broadcastMessage(startMsg.toJson());

  // Broadcast initial game state
  broadcastGameState();

  // Deal cards and send to each active player
  newRound();
}

void Server::messagePlayer(int playerId, const nlohmann::json& message) const {
  if (!running_ || shuttingDown_) {
    log("Server not running, cannot send message");
    return;
  }

  if (playerId < 0 || playerId >= 4) {
    log("Sending message to invalid player ID.");
    return;
  }

  // Get socket pointer under lock to prevent race conditions
  sockpp::tcp_socket* socket = nullptr;
  {
    std::lock_guard<std::mutex> lock(playersMutex_);
    auto& p = players_[playerId];
    if (!p.isActive || !p.socket) {
      log("Sending message to inactive player.");
      return;
    }
    socket = p.socket.get();
  }

  // Send message without holding lock (I/O should not block mutex)
  try {
    std::string data = message.dump() + "\n";
    socket->write(data);

    log("Sending message to " + std::to_string(playerId) + ": " + data);
  } catch (const std::exception& e) {
    logError("Failed to send message to player " + std::to_string(playerId) +
             ": " + e.what());
  }
}

void Server::broadcastMessage(const nlohmann::json& message) const {
  // Collect active player IDs under lock to avoid race conditions with ID
  // reassignment
  std::vector<int> activePlayerIds;
  {
    std::lock_guard<std::mutex> lock(playersMutex_);
    for (const auto& p : players_) {
      if (p.isActive) {
        activePlayerIds.push_back(p.id);
      }
    }
  }

  // Send messages without holding the lock (I/O should not block mutex)
  for (int playerId : activePlayerIds) {
    messagePlayer(playerId, message);
  }
}

void Server::broadcastGameState() const {
  log("Broadcasting game state");

  GameStateUpdateMessage msg(*game_);
  broadcastMessage(msg.toJson());
}

void Server::broadcastPlayerList() const {
  log("Broadcasting player list");

  std::vector<PlayerInfo> playersInfo;

  {
    std::lock_guard<std::mutex> lock(playersMutex_);
    for (const auto& p : players_) {
      if (!p.isActive) continue;

      PlayerInfo info;
      info.id = p.id;
      info.name = p.name;
      info.ready = p.isReady;
      playersInfo.push_back(info);
    }
  }

  PlayerListUpdateMessage msg(playersInfo);
  broadcastMessage(msg.toJson());
}

void Server::log(const std::string& message) const {
  std::cout << "[Server] " << message << std::endl;
}

void Server::logError(const std::string& message) const {
  std::cout << "[ERROR] " << message << std::endl << std::flush;

  std::cerr << "[ERROR] " << message << std::endl << std::flush;
}

// Logs a player's action to the console
void Server::logPlayerAction(int playerId,
                             const nlohmann::json& actionJson) const {
  std::cout << "[Action] Player " << playerId << ": " << actionJson.dump()
            << std::endl;
}
