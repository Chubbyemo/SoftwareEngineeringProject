#ifndef SERVER_HPP
#define SERVER_HPP

#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>

#include <atomic>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "shared/game.hpp"
#include "shared/messages.hpp"

/**
 * @class Server
 * @brief Handles the game server, including client connections and invoking
 * game logic.
 */
class Server {
 public:
  /**
   * @brief Constructs a Server object.
   * @param serverAddress The address of the server.
   * @param port The port number the server listens on.
   * @param connectionTimeout The number of seconds to wait until a connection
   * is considered inactive.
   */
  Server(std::string serverAddress, int port, int connectionTimeout);

  /**
   * @brief Destructs a Server object.
   */
  ~Server();

  /**
   * @brief Starts the server and initializes the game.
   * @throws std::runtime_error if the server fails to start.
   */
  void start();

  /**
   * @brief Stops the server.
   */
  void stop();

  /**
   * @brief Runs the game loop
   */
  void run();

  /**
   * @brief Checks the number of players that are connected to the server.
   * @return Number of currently connected players
   */
  int getNumPlayers() const;

  /**
   * @brief Marks a player as ready.
   * @param playerId The ID of the player
   */
  void setPlayerReady(int playerId);

  /**
   * @brief Checks if all players are ready to start the game.
   * @return True if all players are ready, false otherwise.
   */
  bool areAllPlayersReady() const;

 private:
  /** @brief Player-specific data slot. */
  struct ClientInfo {
    ClientInfo() = default;

    // ENABLE MOVE
    ClientInfo(ClientInfo&&) = default;
    ClientInfo& operator=(ClientInfo&&) = default;

    // DELETE COPY
    ClientInfo(const ClientInfo&) = delete;
    ClientInfo& operator=(const ClientInfo&) = delete;

    int threadId = -1;  ///< ID to identify the listener thread of the client
    int id = -1;        ///< ID of the player
    std::unique_ptr<sockpp::tcp_socket>
        socket;             ///< Connection socket of the client
    std::string name;       ///< Player name.
    bool isActive = false;  ///< Whether the player is connected.
    bool isReady = false;   ///< Whether the player is ready to start.
  };

  sockpp::tcp_acceptor acceptor_;  ///< TCP acceptor for handling connections.

  std::string serverAddress_;  ///< Address of the server.
  int port_;                   ///< Port number for the server.

  mutable std::mutex playersMutex_;  ///< Protects players_ and numPlayers_ from
                                     ///< race conditions

  int numPlayers_ = 0;  ///< Number of players.
  std::array<ClientInfo, 4>
      players_;  ///< Fixed slots for up to 4 players in the game (IDs 0–3).

  static const std::vector<int> idAssignmentOrder;

  std::unique_ptr<BraendiDog::GameState> game_;  ///< Game instance.
  bool gameRunning_ = false;  ///< Flag to control game running status
  bool running_ = true;       ///< Flag to control server status

  std::atomic<bool> shuttingDown_{
      false};  ///< Atomic flag to control server shutdown status

  std::vector<std::thread> clientThreads_;
  std::mutex threadsMutex_;

  std::chrono::seconds
      connectionTimeout_;  ///< Seconds until an unresponsive client is
                           ///< considered disconnected.

  /**
   * @brief Waits for human players to connect.
   * @throws std::runtime_error if connection issues occur.
   */
  void waitForPlayers();

  /**
   * @brief Handles when a new player joins the server.
   * @param sock The connection to the user provided by Sockpp.
   * @return Returns the ID of the newly connected player.
   */
  int handleNewConnection(sockpp::tcp_socket sock);

  /**
   * @brief Handles post-connection workflow (broadcasts player list).
   */
  void onSuccessfulConnection();

  /**
   * @brief Checks if a player's name is non-empty and unique.
   * @param name The name of the player requesting to connect.
   * @return Returns whether the name is valid.
   */
  bool isValidName(const std::string name) const;

  /**
   * @brief Sends a broadcast of the current player list to all active clients.
   */
  void broadcastPlayerList() const;

  /**
   * @brief Checks the connection status of all clients, and updates any
   * disconnected players.
   */
  void checkConnections();

  /**
   * @brief Starts the game when all players are ready.
   */
  void startGame();

  /**
   * @brief Processes actions from a connected player.
   * @param playerId The ID of the player.
   */
  void handleNewMessage(int playerId);

  /**
   * @brief Broadcasts the game state to all connected players.
   */
  void broadcastGameState() const;

  /**
   * @brief Sends a message to a specific player.
   */
  void messagePlayer(int playerId, const nlohmann::json& message) const;

  /**
   * @brief Broadcasts a message to all active players.
   */
  void broadcastMessage(const nlohmann::json& message) const;

  /**
   * @brief Logs general server information or state.
   */
  void log(const std::string& message) const;

  /**
   * @brief Logs error messages to CERR.
   */
  void logError(const std::string& message) const;

  /**
   * @brief Logs a player's action.
   * @param playerId The ID of the player.
   * @param actionJson The JSON representation of the action.
   */
  void logPlayerAction(int playerId, const nlohmann::json& actionJson) const;

  /**
   * @brief Retrieves the names of all active players.
   * @return Array of optional player names indexed by player ID.
   */
  std::array<std::optional<std::string>, 4> getPlayerNames() const;

  /**
   * @brief Handles a client's request to play a card.
   *
   * Validates the move contained in the PlayCardRequestMessage, checks turn
   * order, executes the move if legal, updates the game state, and sends the
   * appropriate response back to the client. If the round or game ends as a
   * result of the move, this function triggers the required round- or game-end
   * logic.
   *
   * @param playerId The ID of the player attempting to play a card.
   * @param req The incoming PlayCardRequestMessage containing the move details.
   */
  void handlePlayCard(size_t handIndex, int playerId,
                      const PlayCardRequestMessage& req);

  /**
   * @brief Handles a player's request to skip their turn.
   * @param playerId The ID of the player requesting to skip their turn.
   */
  void handleSkipTurn(int playerId);

  /**
   * @brief Deals cards to all active players and transmits their hands.
   */
  void dealCards() const;

  /**
   * @brief Starts a new round of the game.
   *
   * Called when the previous round ends or a new game is started. Distributes a
   * fresh set of cards, resets per-round player state, and broadcasts related
   * updates.
   */
  void newRound();

  /**
   * @brief Broadcasts end-of-game results and performs necessary clean up.
   */
  void handleGameEnd();

  /**
   * @brief Updates game state and clients when a client disconnects
   */
  void handleDisconnect(const size_t playerId);
};

#endif  // SERVER_HPP
