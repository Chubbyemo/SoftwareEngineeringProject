#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "game.hpp"
#include "game_types.hpp"

/**
 * @brief Enumeration of all message types in the client-server communication.
 */
enum class MessageType {
  // Server-to-Client responses
  RESP_CONNECT,     ///< Response to connection request --MessageType 0
  RESP_READY,       ///< Response to ready status update --MessageType 1
  RESP_START_GAME,  ///< Response to start game request --MessageType 2
  RESP_PLAY_CARD,   ///< Response to play card request --MessageType 3
  RESP_SKIP_TURN,   ///< Response to skip turn request --MessageType 4

  // Server broadcast messages
  BRDC_PLAYER_LIST,  ///< Notification of player joining --MessageType 6
  BRDC_GAME_START,   ///< Notification that game is starting --MessageType 7
  BRDC_GAMESTATE_UPDATE,  ///< Broadcast of updated game state --MessageType 8
  BRDC_PLAYER_DISCONNECTED,  ///< Notification of player disconnecting
                             ///< --MessageType 9
  BRDC_PLAYER_FINISHED,  ///< Notification of player finishing --MessageType 10
  BRDC_RESULTS,          ///< Notification of game results --MessageType 11

  // Server private messages
  PRIV_CARDS_DEALT,  ///< Private message with dealt cards --MessageType 12

  // Client-to-Server requests
  REQ_CONNECT,     ///< Connection request --MessageType 13
  REQ_READY,       ///< Ready status request --MessageType 14
  REQ_START_GAME,  ///< Start game request --MessageType 15
  REQ_PLAY_CARD,   ///< Game move request --MessageType 16
  REQ_SKIP_TURN,   ///< Forced fold request --MessageType 17
};

/**
 * @brief Converts MessageType enum to string for JSON serialization.
 * @param type The MessageType to convert.
 * @return String representation of the message type.
 */
inline std::string messageTypeToString(MessageType type) {
  switch (type) {
    case MessageType::RESP_CONNECT:
      return "RESP_CONNECT";
    case MessageType::RESP_READY:
      return "RESP_READY";
    case MessageType::RESP_START_GAME:
      return "RESP_START_GAME";
    case MessageType::RESP_PLAY_CARD:
      return "RESP_PLAY_CARD";
    case MessageType::RESP_SKIP_TURN:
      return "RESP_SKIP_TURN";

    case MessageType::BRDC_PLAYER_LIST:
      return "BRDC_PLAYER_LIST";
    case MessageType::BRDC_GAME_START:
      return "BRDC_GAME_START";
    case MessageType::BRDC_GAMESTATE_UPDATE:
      return "BRDC_GAMESTATE_UPDATE";
    case MessageType::BRDC_PLAYER_DISCONNECTED:
      return "BRDC_PLAYER_DISCONNECTED";
    case MessageType::BRDC_PLAYER_FINISHED:
      return "BRDC_PLAYER_FINISHED";
    case MessageType::BRDC_RESULTS:
      return "BRDC_RESULTS";

    case MessageType::PRIV_CARDS_DEALT:
      return "PRIV_CARDS_DEALT";

    case MessageType::REQ_CONNECT:
      return "REQ_CONNECT";
    case MessageType::REQ_READY:
      return "REQ_READY";
    case MessageType::REQ_START_GAME:
      return "REQ_START_GAME";
    case MessageType::REQ_PLAY_CARD:
      return "REQ_PLAY_CARD";
    case MessageType::REQ_SKIP_TURN:
      return "REQ_SKIP_TURN";
  }
  std::cerr << "Unknown MessageType: " << static_cast<int>(type) << std::endl;
  std::abort();
}

/**
 * @brief Converts string to MessageType enum for JSON deserialization.
 * @param s The string to convert.
 * @return MessageType enum value.
 * @note This function will crash if given an unknown string - all valid message
 * types must be handled.
 */
inline MessageType stringToMessageType(const std::string& s) {
  if (s == "RESP_CONNECT")
    return MessageType::RESP_CONNECT;
  else if (s == "RESP_READY")
    return MessageType::RESP_READY;
  else if (s == "RESP_START_GAME")
    return MessageType::RESP_START_GAME;
  else if (s == "RESP_PLAY_CARD")
    return MessageType::RESP_PLAY_CARD;
  else if (s == "RESP_SKIP_TURN")
    return MessageType::RESP_SKIP_TURN;

  else if (s == "BRDC_PLAYER_LIST")
    return MessageType::BRDC_PLAYER_LIST;
  else if (s == "BRDC_GAME_START")
    return MessageType::BRDC_GAME_START;
  else if (s == "BRDC_GAMESTATE_UPDATE")
    return MessageType::BRDC_GAMESTATE_UPDATE;
  else if (s == "BRDC_PLAYER_DISCONNECTED")
    return MessageType::BRDC_PLAYER_DISCONNECTED;
  else if (s == "BRDC_PLAYER_FINISHED")
    return MessageType::BRDC_PLAYER_FINISHED;
  else if (s == "BRDC_RESULTS")
    return MessageType::BRDC_RESULTS;

  else if (s == "PRIV_CARDS_DEALT")
    return MessageType::PRIV_CARDS_DEALT;

  else if (s == "REQ_CONNECT")
    return MessageType::REQ_CONNECT;
  else if (s == "REQ_READY")
    return MessageType::REQ_READY;
  else if (s == "REQ_START_GAME")
    return MessageType::REQ_START_GAME;
  else if (s == "REQ_PLAY_CARD")
    return MessageType::REQ_PLAY_CARD;
  else if (s == "REQ_SKIP_TURN")
    return MessageType::REQ_SKIP_TURN;

  // Unknown string - crash with informative message
  std::cerr << "FATAL ERROR in stringToMessageType(): Unknown msgType string: '"
            << s << "'" << std::endl;
  std::abort();
}

// *** Abstract Base Classes ***

/**
 * @brief Base class for all messages in the client-server communication.
 *
 * Provides common functionality for message type identification and JSON
 * serialization. All messages contain a discriminator field to identify the
 * concrete message type.
 */
class Message {
 public:
  /**
   * @brief Virtual destructor for proper inheritance cleanup.
   */
  virtual ~Message() = default;

  /**
   * @brief Gets the message type identifier.
   * @return MessageType enum identifying the specific message type.
   */
  virtual MessageType getMessageType() const = 0;

  /**
   * @brief Serializes the message to JSON.
   * @return JSON representation of the message.
   */
  virtual nlohmann::json toJson() const = 0;

  /**
   * @brief Creates a Message instance from JSON data.
   * @param json The JSON data to parse.
   * @return Unique pointer to the created message, or nullptr if parsing fails.
   */
  static std::unique_ptr<Message> fromJson(const nlohmann::json& json);

  /**
   * @brief Template method to deserialize any message type from JSON.
   * @param json The JSON data to parse.
   * @return Unique pointer to the created message. Crashes if parsing fails.
   */
  template <typename T>
  static std::unique_ptr<T> fromJsonImpl(const nlohmann::json& json) {
    try {
      auto msg = std::make_unique<T>();
      json.get_to(*msg);
      return msg;
    } catch (const std::exception& e) {
      std::cerr << "FATAL ERROR in Message::fromJsonImpl(): Failed to "
                   "deserialize message of type "
                << typeid(T).name() << " from JSON: " << json.dump()
                << " Error: " << e.what() << std::endl;
      std::abort();
    }
  }

  /**
   * @brief Returns a readable string version of the message.
   * @param indent Number of spaces to indent.
   * @return A formatted JSON string of the message.
   */
  std::string toString(int indent = 2) const {
    try {
      return toJson().dump(indent);
    } catch (const std::exception& e) {
      std::cerr << "Error serializing message to string: " << e.what()
                << std::endl;
      return "{}";
    }
  }

 protected:
  /**
   * @brief Helper to add the message type to JSON.
   * @param json The JSON object to modify.
   * @param messageType The message type to add.
   */
  static void addMessageType(nlohmann::json& json, MessageType messageType) {
    json["msgType"] = messageTypeToString(messageType);
  }
};

/**
 * @brief Macro to implement standard toJson() method for messages with fields.
 */
#define IMPLEMENT_MESSAGE_TOJSON()          \
  nlohmann::json toJson() const override {  \
    nlohmann::json json;                    \
    addMessageType(json, getMessageType()); \
    nlohmann::json data = *this;            \
    json.update(data);                      \
    return json;                            \
  }

/**
 * @brief Macro to implement standard toJson() method for messages without
 * fields.
 */
#define IMPLEMENT_EMPTY_MESSAGE_TOJSON()    \
  nlohmann::json toJson() const override {  \
    nlohmann::json json;                    \
    addMessageType(json, getMessageType()); \
    return json;                            \
  }

/**
 * @brief Abstract base class for client-to-server request messages that include
 * a player ID (REQ_START_GAME).
 *
 * Note: REQ_CONNECT does NOT inherit from this class as it doesn't have a
 * playerId initially (playerId is assigned by the server).
 */
class ClientRequest : public Message {
 protected:
  MessageType msgType_;
  size_t playerId_;  ///< ID of the player sending this request

 public:
  /**
   * @brief Constructs a client request message.
   * @param t Message type
   * @param id ID of the player sending the request
   */
  ClientRequest(MessageType t, size_t id) : msgType_(t), playerId_(id) {}
  ClientRequest() = default;

  /**
   * @brief Gets the player ID of the sender.
   * @return Player ID
   */
  size_t getPlayerId() const { return playerId_; }

  /**
   * @brief Gets the type of this message.
   * @return MessageType of the request
   */
  MessageType getMessageType() const override { return msgType_; }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClientRequest, playerId_)
};

/**
 * @brief Abstract base class for all server-to-client response messages
 * (RESP_*). Responses include success or failure of client requests.
 */
class ServerResponse : public Message {
 protected:
  MessageType msgType_;
  bool success_ = true;  ///< Whether the request was successfully processed
  std::string errorMsg_ = "";  ///< Error message if the request failed

 public:
  /**
   * @brief Constructs a server response.
   * @param t Message type
   * @param success Whether the response indicates success
   * @param errorMsg Optional error message (empty if success)
   */
  ServerResponse(MessageType t, bool success, std::string errorMsg = "")
      : msgType_(t), success_(success), errorMsg_(std::move(errorMsg)) {}
  ServerResponse() = default;

  /**
   * @brief Checks if the request was successful.
   * @return True if successful, false otherwise
   */
  bool getSuccess() const { return success_; }

  /**
   * @brief Returns the error message if the response failed.
   * @return Error message string
   */
  std::string getErrorMsg() const { return errorMsg_; }

  /**
   * @brief Gets the message type.
   * @return MessageType of the response
   */
  MessageType getMessageType() const override { return msgType_; }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ServerResponse, success_, errorMsg_)
};

/**
 * @brief Abstract base class for all server-to-all-clients broadcast messages
 * (BRDC_*). Used for updates that all players should receive.
 */
class BroadcastMessage : public Message {
 protected:
  MessageType msgType_;

 public:
  /**
   * @brief Constructs a broadcast message.
   * @param t Message type
   */
  BroadcastMessage(MessageType t) : msgType_(t) {}
  BroadcastMessage() = default;

  /**
   * @brief Gets the message type.
   * @return MessageType of the broadcast
   */
  MessageType getMessageType() const override { return msgType_; }

  IMPLEMENT_EMPTY_MESSAGE_TOJSON()
};

/**
 * @brief Abstract base class for all server-to-specific-client messages
 * (PRIV_*). Used for messages that only one player should receive (e.g., dealt
 * cards).
 */
class PrivateMessage : public Message {
 protected:
  MessageType msgType_;
  size_t playerId_;  ///< ID of the recipient player

 public:
  /**
   * @brief Constructs a private message.
   * @param t Message type
   * @param id Player ID of the recipient
   */
  PrivateMessage(MessageType t, size_t id) : msgType_(t), playerId_(id) {}
  PrivateMessage() = default;

  /**
   * @brief Gets the player ID of the recipient.
   * @return Player ID
   */
  size_t getPlayerId() const { return playerId_; }

  /**
   * @brief Gets the message type.
   * @return MessageType of the private message
   */
  MessageType getMessageType() const override { return msgType_; }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PrivateMessage, playerId_)
};

// ==================== CLIENT-TO-SERVER MESSAGES ====================

/**
 * @brief Client request to connect to the server.
 *
 * Note: This does NOT inherit from ClientRequest because it doesn't have a
 * playerId initially - the playerId is assigned by the server in RESP_CONNECT.
 */
class ConnectionRequestMessage : public Message {
 public:
  std::string name;  ///< Player's display name

  ConnectionRequestMessage(std::string name) : name(std::move(name)) {}
  ConnectionRequestMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::REQ_CONNECT;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConnectionRequestMessage, name)
};

/**
 * @brief Client request to mark player as ready.
 */
class ReadyMessage : public ClientRequest {
 public:
  ReadyMessage(size_t id) : ClientRequest(MessageType::REQ_READY, id) {}
  ReadyMessage() = default;

  MessageType getMessageType() const override { return MessageType::REQ_READY; }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReadyMessage, playerId_)
};

/**
 * @brief Client request to start a new game.
 */
class StartGameRequestMessage : public ClientRequest {
 public:
  StartGameRequestMessage(size_t id)
      : ClientRequest(MessageType::REQ_START_GAME, id) {}
  StartGameRequestMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::REQ_START_GAME;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(StartGameRequestMessage, playerId_)
};

// ==================== SERVER-TO-CLIENT MESSAGES ====================

/**
 * @brief Player status information for player list updates.
 */
struct PlayerInfo {
  size_t id;
  std::string name;
  bool ready = false;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerInfo, id, name, ready)
};

/**
 * @brief Server response acknowledging a connection attempt.
 */
class ConnectionResponseMessage : public ServerResponse {
 public:
  size_t playerId;  ///< Assigned player ID (only if success is true)
  ConnectionResponseMessage(bool success, std::string err, size_t id)
      : ServerResponse(MessageType::RESP_CONNECT, success, std::move(err)),
        playerId(id) {}
  ConnectionResponseMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::RESP_CONNECT;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConnectionResponseMessage, success_, errorMsg_,
                                 playerId)
};

/**
 * @brief Server response confirming the start of a game.
 */
class ReadyResponseMessage : public ServerResponse {
 public:
  ReadyResponseMessage(bool success, std::string err = "")
      : ServerResponse(MessageType::RESP_READY, success, std::move(err)) {}
  ReadyResponseMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::RESP_READY;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReadyResponseMessage, success_, errorMsg_)
};

/**
 * @brief Server response confirming the start of a game.
 */
class StartGameResponseMessage : public ServerResponse {
 public:
  StartGameResponseMessage(bool success, std::string err = "")
      : ServerResponse(MessageType::RESP_START_GAME, success, std::move(err)) {}
  StartGameResponseMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::RESP_START_GAME;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(StartGameResponseMessage, success_, errorMsg_)
};

// ==================== SERVER BROADCAST MESSAGES ====================

/**
 * @brief Broadcast message updating the player list when a player joins.
 *
 * Note: Uses map<size_t, string> per spec (playerId to username mapping)
 */
class PlayerListUpdateMessage : public BroadcastMessage {
 public:
  std::vector<PlayerInfo> playersList;  ///< Map of playerId to username
  PlayerListUpdateMessage(std::vector<PlayerInfo> playersInfo)
      : BroadcastMessage(MessageType::BRDC_PLAYER_LIST),
        playersList(std::move(playersInfo)) {}
  PlayerListUpdateMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::BRDC_PLAYER_LIST;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerListUpdateMessage, playersList)
};

/**
 * @brief Broadcast announcing game start.
 */
class GameStartMessage : public BroadcastMessage {
 public:
  size_t numPlayers;
  GameStartMessage(int numPlayers)
      : BroadcastMessage(MessageType::BRDC_GAME_START),
        numPlayers(numPlayers) {}

  GameStartMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::BRDC_GAME_START;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameStartMessage, numPlayers)
};

/**
 * @brief Broadcast notifying that a player disconnected.
 */
class PlayerDisconnectedMessage : public BroadcastMessage {
 public:
  size_t playerId;  ///< ID of the player who disconnected
  PlayerDisconnectedMessage(size_t id)
      : BroadcastMessage(MessageType::BRDC_PLAYER_DISCONNECTED), playerId(id) {}
  PlayerDisconnectedMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::BRDC_PLAYER_DISCONNECTED;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerDisconnectedMessage, playerId)
};

/**
 * @brief Client request to play a card and execute a move.
 *
 * NOTE: Passes Move object (defined in game_types.hpp) to specify the move
 * details.
 */
class PlayCardRequestMessage : public ClientRequest {
 public:
  BraendiDog::Move move;

  PlayCardRequestMessage(size_t playerId, BraendiDog::Move& m)
      : ClientRequest(MessageType::REQ_PLAY_CARD, playerId), move(m) {}
  PlayCardRequestMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::REQ_PLAY_CARD;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayCardRequestMessage, playerId_, move)
};

/**
 * @brief Client request to skip a turn.
 */
class SkipTurnRequestMessage : public ClientRequest {
 public:
  SkipTurnRequestMessage(size_t id)
      : ClientRequest(MessageType::REQ_SKIP_TURN, id) {}
  SkipTurnRequestMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::REQ_SKIP_TURN;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SkipTurnRequestMessage, playerId_)
};

/**
 * @brief Server response to a card play action.
 */
class PlayCardResponseMessage : public ServerResponse {
 public:
  size_t handIndex;  ///< Index of the played card in the player's hand

  PlayCardResponseMessage(size_t handIndex, bool success, std::string err = "")
      : ServerResponse(MessageType::RESP_PLAY_CARD, success, std::move(err)),
        handIndex(handIndex) {}
  PlayCardResponseMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::RESP_PLAY_CARD;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayCardResponseMessage, handIndex, success_,
                                 errorMsg_)
};

/**
 * @brief Server response to a skip turn request.
 */
class SkipTurnResponseMessage : public ServerResponse {
 public:
  SkipTurnResponseMessage(bool success, std::string err = "")
      : ServerResponse(MessageType::RESP_SKIP_TURN, success, std::move(err)) {}
  SkipTurnResponseMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::RESP_SKIP_TURN;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SkipTurnResponseMessage, success_, errorMsg_)
};

/**
 * @brief Broadcast containing updated game state.
 */
class GameStateUpdateMessage : public BroadcastMessage {
 public:
  BraendiDog::GameState gameState;

  GameStateUpdateMessage() = default;

  GameStateUpdateMessage(const BraendiDog::GameState& state)
      : BroadcastMessage(MessageType::BRDC_GAMESTATE_UPDATE),
        gameState(state) {}

  MessageType getMessageType() const override {
    return MessageType::BRDC_GAMESTATE_UPDATE;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameStateUpdateMessage, gameState)
};

/**
 * @brief Broadcast notifying that a player has finished.
 */
class PlayerFinishedMessage : public BroadcastMessage {
 public:
  size_t playerId;
  // TODO: Add fields: playerId, username, finishPosition, elapsedTime
  PlayerFinishedMessage(const size_t playerId)
      : BroadcastMessage(MessageType::BRDC_PLAYER_FINISHED),
        playerId(playerId) {}
  PlayerFinishedMessage() = default;
  MessageType getMessageType() const override {
    return MessageType::BRDC_PLAYER_FINISHED;
  }
  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerFinishedMessage, playerId)
};

/**
 * @brief Broadcast with final game results.
 * TODO: Implement according to spec section 4.1.17
 */
class GameResultsMessage : public BroadcastMessage {
 public:
  std::array<std::optional<int>, 4> rankings;
  GameResultsMessage(const std::array<std::optional<int>, 4>& rankings)
      : BroadcastMessage(MessageType::BRDC_RESULTS), rankings(rankings) {}
  GameResultsMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::BRDC_RESULTS;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameResultsMessage, rankings)
};

/**
 * @brief Private message delivering cards to a specific player.
 */
class CardsDealtMessage : public PrivateMessage {
 public:
  // TODO: Add field: cards (vector<size_t>)
  std::vector<size_t> cards;

  CardsDealtMessage(size_t id, std::vector<size_t> cards)
      : PrivateMessage(MessageType::PRIV_CARDS_DEALT, id),
        cards(std::move(cards)) {}
  CardsDealtMessage() = default;

  MessageType getMessageType() const override {
    return MessageType::PRIV_CARDS_DEALT;
  }

  IMPLEMENT_MESSAGE_TOJSON()
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(CardsDealtMessage, playerId_, cards)
};
