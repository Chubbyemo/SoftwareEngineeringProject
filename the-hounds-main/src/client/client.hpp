#pragma once

#include <sockpp/tcp_connector.h>

#include <functional>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <thread>

#include "shared/game_types.hpp"

/**
 * @struct Player
 * @brief Represents a player in the game, including their name, and readiness
 * (not game logic)
 */
struct PlayerStatus {
  size_t id;         ///< ID of the player.
  std::string name;  ///< Name of the player.
  bool isReady;      ///< Whether the player is ready.
};

/**
 * @class Client
 * @brief Handles the client-side networking and communication with the server.
 */
class Client {
 public:
  /**
   * @brief Constructs a Client object and connects to the server.
   * @param serverAddress IP address of the server.
   * @param port Port number to connect to.
   * @param playerName Name of the player.
   * @throws std::runtime_error if connection to the server fails.
   */
  Client(const std::string& serverAddress, const int port,
         const std::string playerName);

  /**
   * @brief Destructor to properly clean up the client.
   */
  ~Client();

  /**
   * @brief Sends an action to the server.
   * @param actionJson JSON object representing the action.
   */
  void sendAction(nlohmann::json& actionJson);

  /**
   * @brief Signals that the client is transitioning from lobby to game.
   *        All subsequent messages will be buffered until game is ready.
   */
  void beginTransitionToGame();

  /**
   * @brief Signals that MainGameFrame is ready to receive messages.
   *        Flushes buffered messages and switches to game mode.
   */
  void completeTransitionToGame();

  /**
   * @brief Sets a callback function to handle incoming server messages.
   * @param callback Function to call when new data is received.
   */
  void setUpdateCallback(std::function<void(const std::string&)> callback);

  //// Client Action Methods ////

  /**
   * @brief Sends a ready signal to the server.
   */
  void sendReady();

  /**
   * @brief Sends the start signal to the server once all players are ready.
   */
  void sendStartGame();

  /**
   * @brief Sends a play card request to the server with move details.
   * @param move The Move structure detailing the marbles to be moved.
   */
  void sendPlayCard(BraendiDog::Move move);

  /**
   * @brief Sends a skip turn request to the server.
   */
  void sendSkipTurn();

  //// Getters ////

  /**
   * @brief Gets the player's assigned index from the server.
   * @return Player index as an integer.
   */
  int getPlayerIndex() const;

  /**
   * @brief Gets the player's name.
   * @return Player name as a string.
   */
  std::string getPlayerName() const;

  /**
   * @brief Gets the list of players and their readiness status.
   * @return Vector of player structs containing name and readiness status.
   */
  std::array<PlayerStatus, 4> getPlayerList() const;

  //// Setters ////

  /**
   * @brief Sets the player's assigned index from the server.
   * @param id The id the Server assigned to the player.
   */
  void setPlayerIndex(size_t id);

  /**
   * @brief Checks if all players are ready to start the game.
   * @return True if all players are ready, false otherwise.
   */
  bool areAllPlayersReady() const;

 private:
  // State machine for message routing
  enum class ClientState {
    LOBBY,          // In lobby, messages go to LobbyFrame
    TRANSITIONING,  // Switching to game, buffer messages
    GAME            // In game, messages go to MainGameFrame
  };
  sockpp::tcp_connector connection;  ///< TCP connection to the server.
  std::thread listenerThread;        ///< Thread to listen for server messages.

  std::string initialBuffer_;  // Store any messages received during connection
  ClientState state_ = ClientState::LOBBY;
  std::queue<std::string>
      transitionBuffer_;  // Buffer messages during transition

  std::function<void(const std::string&)>
      updateCallback;  ///< Callback function for received messages.
  std::queue<std::string> pendingMessages_;  ///< Stores pending messages if
                                             ///< callback is not yet set.
  bool running = true;  ///< Flag for controlling the listener thread loop.

  int playerIndex = -1;  ///< Player's assigned index from the server.
  std::string playerName;
  std::array<PlayerStatus, 4> playerList_;  ///< Local representation of player
                                            ///< list with readiness status.

  /**
   * @brief Continuously listens for messages from the server.
   */
  void ServerListener();

  /**
   * @brief Centralized handler for parsing and acting on server messages.
   * @param message The JSON message received from the server.
   */
  void handleServerMessage(const nlohmann::json& message);

  /**
   * @brief Updates the UI via the GUI callback.
   * @param message The raw message to send to the GUI.
   */
  void notifyUpdate(const std::string& message);
};
