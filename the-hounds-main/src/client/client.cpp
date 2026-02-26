#include "client/client.hpp"

#include <sockpp/tcp_connector.h>

#include <functional>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>

#include "shared/game_types.hpp"
#include "shared/messages.hpp"

// Constructor: Establishes a connection to the server
Client::Client(const std::string& serverAddress, const int port,
               const std::string playerName)
    : playerName(playerName) {
  // Attempt to connect to the server
  connection =
      sockpp::tcp_connector({serverAddress, static_cast<in_port_t>(port)});
  if (!connection) {
    throw std::runtime_error("Failed to connect to the server");
  }
  std::cout << "Connected to Server." << std::endl;

  // Send connection request (REQ_CONNECT) and receive response (RESP_CONNECT)
  ConnectionRequestMessage connReq(playerName);
  std::string connReqJson = connReq.toJson().dump() + "\n";
  if (connection.write(connReqJson) != connReqJson.size()) {
    throw std::runtime_error("Failed to send connection request to server");
  }

  // Buffer to receive initial data from server
  char buf[1024];
  // Read the response from the server
  ssize_t n = connection.read(buf, sizeof(buf));
  if (n <= 0) {
    throw std::runtime_error("Failed to receive connection response");
  }
  // Extract message (RESP_CONNECT) and handle newline
  std::string responseStr(buf, n);
  size_t pos = responseStr.find('\n');
  std::string firstMessage = responseStr.substr(0, pos);

  // Pars
  nlohmann::json responseJson = nlohmann::json::parse(firstMessage);
  auto responseMessage = Message::fromJson(responseJson);
  auto* connResponse =
      static_cast<ConnectionResponseMessage*>(responseMessage.get());

  // Check success of connection
  if (!connResponse->getSuccess()) {
    throw std::runtime_error("Connection rejected by server: " +
                             connResponse->getErrorMsg());
  }
  playerIndex = connResponse->playerId;
  std::cout << "Got Player Index: " << playerIndex << std::endl;

  // Handle remaining messages from server (e.g., initial player list)
  std::string remainder =
      responseStr.substr(pos + 1);  // everything after first \n
  if (!remainder.empty()) {
    // Store remainder for listener thread to process first
    initialBuffer_ = remainder;
  }

  // Start the listener thread to receive messages from the server
  listenerThread = std::thread(&Client::ServerListener, this);
}

// Destructor: Ensures the listener thread is properly terminated
Client::~Client() {
  if (listenerThread.joinable()) {
    listenerThread.join();
  }
}

//// Interface Methods ////

// Listener thread function to receive messages from the server
void Client::ServerListener() {
  std::string buffer;  // Accumulate data here

  // If there's initial buffered data, process it first
  if (!initialBuffer_.empty()) {
    buffer += initialBuffer_;
    initialBuffer_.clear();
  }

  char buf[4096];
  while (running) {  // Ensure the thread respects the running flag
    try {
      // Process complete messages (newline-delimited)
      size_t pos;
      while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string message = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);  // Remove processed message

        if (!message.empty()) {
          nlohmann::json receivedJson = nlohmann::json::parse(message);
          handleServerMessage(receivedJson);
        }
      }

      ssize_t n = connection.read(buf, sizeof(buf));
      if (n <= 0) {
        notifyUpdate("");  // Notify GUI of a potential disconnect
        break;
      }

      // Append new data to buffer
      buffer.append(buf, n);
    } catch (const std::exception& ex) {
      std::cerr << "Exception in ServerListener: " << ex.what() << std::endl;
      break;
    }
  }

  std::cout << "Listener thread exiting..." << std::endl;
  running = false;  // Securely stop the loop
}

// Centralized server message handler
void Client::handleServerMessage(const nlohmann::json& message) {
  try {
    auto parsedMessage = Message::fromJson(message);
    MessageType messageType = parsedMessage->getMessageType();

    // Route based on client state
    switch (state_) {
      case ClientState::LOBBY:
      case ClientState::GAME:
        // Normal operation: deliver message immediately
        break;

      case ClientState::TRANSITIONING:
        // Buffer ALL messages during transition
        transitionBuffer_.push(message.dump());
        std::cout << "Buffered message during transition: "
                  << messageTypeToString(messageType) << std::endl;
        return;  // Don't process now, will be flushed later
    }

    switch (messageType) {
      // BRDC_* messages from server
      case MessageType::BRDC_PLAYER_LIST: {
        auto* playerListMessage =
            static_cast<PlayerListUpdateMessage*>(parsedMessage.get());

        // Clear the current list
        playerList_.fill(
            PlayerStatus{});  // fills all 4 slots of the array with a
                              // default-constructed PlayerStatus
                              // -> name: empty string, isReady: false

        for (const auto& playerInfo : playerListMessage->playersList) {
          PlayerStatus player;
          player.id = playerInfo.id;
          player.name = playerInfo.name;
          player.isReady = playerInfo.ready;

          playerList_[playerInfo.id] =
              player;  // the list is in order of the players' IDs now
        }
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::BRDC_GAME_START: {
        std::cout << "Game started by server!" << std::endl;
        // CRITICAL: Start buffering immediately on this thread before notifying
        // GUI This ensures GAMESTATE_UPDATE and CARDS_DEALT messages that
        // arrive right after this one will be buffered instead of delivered to
        // lobby
        beginTransitionToGame();
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::BRDC_GAMESTATE_UPDATE: {
        std::cout << "GameStateUpdate triggered \n";
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::BRDC_PLAYER_DISCONNECTED: {
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::BRDC_PLAYER_FINISHED: {
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::BRDC_RESULTS: {
        notifyUpdate(message.dump());
        break;
      }
      // PRIV_* messages from server
      case MessageType::PRIV_CARDS_DEALT: {
        notifyUpdate(message.dump());
        break;
      }
      // RESP_* messages from server
      case MessageType::RESP_START_GAME: {
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::RESP_PLAY_CARD: {
        notifyUpdate(message.dump());
        break;
      }
      case MessageType::RESP_SKIP_TURN: {
        notifyUpdate(message.dump());
        break;
      }
      // Unexpected message types from server (all REQ_*)
      case MessageType::REQ_CONNECT:
      case MessageType::REQ_READY:
      case MessageType::RESP_READY:
      case MessageType::REQ_START_GAME:
      case MessageType::REQ_PLAY_CARD:
      case MessageType::REQ_SKIP_TURN:
      case MessageType::RESP_CONNECT:  // Should not be received here only in
                                       // constructor
        std::cerr << "Unexpected message type from server: "
                  << static_cast<int>(messageType)
                  << messageTypeToString(messageType) << std::endl;
        break;
    }

  } catch (const std::exception& ex) {
    std::cerr << "Error handling server message: " << ex.what() << std::endl;
  }
}

// Signal beginning of transition from lobby to game
void Client::beginTransitionToGame() {
  std::cout << "Beginning transition to game - buffering messages" << std::endl;
  state_ = ClientState::TRANSITIONING;
}

// Signal that MainGameFrame is ready and flush buffered messages
void Client::completeTransitionToGame() {
  std::queue<std::string> reordered;
  std::optional<std::string> gameStateMsg;

  std::cout << "=== BUFFER CONTENTS ===" << std::endl << std::flush;

  while (!transitionBuffer_.empty()) {
    std::string msg = transitionBuffer_.front();
    transitionBuffer_.pop();

    // DEBUG: Print what we're looking at
    std::cout << "Checking message: " << msg.substr(0, 100) << "..."
              << std::endl
              << std::flush;

    // Check if this is the GAMESTATE_UPDATE message
    if (msg.find("\"msgType\":\"BRDC_GAMESTATE_UPDATE\"") !=
        std::string::npos) {
      std::cout << "FOUND GAMESTATE_UPDATE!" << std::endl << std::flush;
      gameStateMsg = msg;
    } else {
      std::cout << "Not GAMESTATE_UPDATE, adding to reordered queue"
                << std::endl
                << std::flush;
      reordered.push(msg);
    }
  }

  std::cout << "=== END BUFFER ===" << std::endl << std::flush;
  std::cout << "gameStateMsg has value: "
            << (gameStateMsg.has_value() ? "YES" : "NO") << std::endl
            << std::flush;
  std::cout << "updateCallback is null: " << (updateCallback ? "NO" : "YES")
            << std::endl
            << std::flush;

  state_ = ClientState::GAME;

  if (gameStateMsg.has_value() && updateCallback) {
    std::cout
        << "Processing GAMESTATE_UPDATE first (reordered for initialization)"
        << std::endl
        << std::flush;
    updateCallback(gameStateMsg.value());
    std::cout << "GAMESTATE_UPDATE processed successfully" << std::endl
              << std::flush;
  } else {
    std::cerr << "WARNING: No GAMESTATE_UPDATE found in transition buffer!"
              << std::endl
              << std::flush;
  }

  // Process remaining messages in original order
  std::cout << "Processing " << reordered.size()
            << " remaining buffered messages" << std::endl
            << std::flush;
  while (!reordered.empty() && updateCallback) {
    std::string msg = reordered.front();
    std::cout << "Processing buffered message: " << msg.substr(0, 50) << "..."
              << std::endl
              << std::flush;
    updateCallback(msg);
    reordered.pop();
  }
  std::cout << "All buffered messages processed" << std::endl << std::flush;
}

// Notify the GUI of an incoming server message
void Client::notifyUpdate(const std::string& message) {
  if (updateCallback) {
    // Check if there's a valid callback set
    updateCallback(message);
  } else {
    // If no callback, store the message for later delivery
    pendingMessages_.push(message);
  }
}

// Sends a JSON action to the server
void Client::sendAction(nlohmann::json& actionJson) {
  std::string message = actionJson.dump() + "\n";
  if (connection.write(message) != message.size()) {
    throw std::runtime_error("Failed to send action to server");
  }
}

// Sets a callback function for receiving updates from the server
void Client::setUpdateCallback(
    std::function<void(const std::string&)> callback) {
  updateCallback = callback;

  // Process any pending messages
  while (!pendingMessages_.empty()) {
    updateCallback(pendingMessages_.front());
    pendingMessages_.pop();
  }
}

//// Getters ////

// Retrieves the assigned player index from the server
int Client::getPlayerIndex() const { return playerIndex; }

// Retrieves the current list of players and their statuses
std::array<PlayerStatus, 4> Client::getPlayerList() const {
  return playerList_;
}

std::string Client::getPlayerName() const { return playerName; }

// Checks if all players are marked as ready
bool Client::areAllPlayersReady() const {
  std::cout << "Checking if all players are ready" << std::endl;
  for (const auto& player : playerList_) {
    if (!player.name.empty() && !player.isReady) {
      return false;
    }
  }
  return true;
}

//// Setters ////
void Client::setPlayerIndex(size_t id) {
  std::cout << "Setting player index from " << playerIndex << " to " << id
            << std::endl;
  playerIndex = id;
}

//// Client Action Methods ////

// Sends a "ready" command to indicate the player is ready to play
void Client::sendReady() {
  ReadyMessage message = ReadyMessage(playerIndex);
  nlohmann::json actionJson = message.toJson();
  sendAction(actionJson);
}

// Sends a request to start the game and switch to game panel
void Client::sendStartGame() {
  StartGameRequestMessage message(playerIndex);
  nlohmann::json actionJson = message.toJson();
  sendAction(actionJson);
}

// Sends a playCard command to play a specific move to the server
void Client::sendPlayCard(BraendiDog::Move move) {
  PlayCardRequestMessage message(playerIndex, move);
  nlohmann::json actionJson = message.toJson();
  sendAction(actionJson);
}

// Send skip turn request to server
void Client::sendSkipTurn() {
  SkipTurnRequestMessage message(playerIndex);
  nlohmann::json actionJson = message.toJson();
  sendAction(actionJson);
}
