#include "messages.hpp"

#include <iostream>

std::unique_ptr<Message> Message::fromJson(const nlohmann::json& json) {
  try {
    // Check for both "msgType" (spec) and "action" (legacy) for backward
    // compatibility
    std::string msgTypeStr;
    if (json.contains("msgType")) {
      msgTypeStr = json.at("msgType").get<std::string>();
    } else if (json.contains("action")) {
      msgTypeStr = json.at("action").get<std::string>();
    } else {
      std::cerr << "FATAL ERROR in Message::fromJson(): Message missing "
                   "'msgType' or 'action' field in JSON: "
                << json.dump() << std::endl;
      std::abort();
    }

    MessageType messageType = stringToMessageType(msgTypeStr);

    switch (messageType) {
      // Client-to-Server requests
      case MessageType::REQ_CONNECT:
        return Message::fromJsonImpl<ConnectionRequestMessage>(json);
      case MessageType::REQ_READY:
        return Message::fromJsonImpl<ReadyMessage>(json);
      case MessageType::REQ_START_GAME:
        return Message::fromJsonImpl<StartGameRequestMessage>(json);
      case MessageType::REQ_PLAY_CARD:
        return Message::fromJsonImpl<PlayCardRequestMessage>(json);
      case MessageType::REQ_SKIP_TURN:
        return Message::fromJsonImpl<SkipTurnRequestMessage>(json);

      // Server-to-Client responses
      case MessageType::RESP_CONNECT:
        return Message::fromJsonImpl<ConnectionResponseMessage>(json);
      case MessageType::RESP_READY:
        return Message::fromJsonImpl<ReadyResponseMessage>(json);
      case MessageType::RESP_START_GAME:
        return Message::fromJsonImpl<StartGameResponseMessage>(json);
      case MessageType::RESP_PLAY_CARD:
        return Message::fromJsonImpl<PlayCardResponseMessage>(json);
      case MessageType::RESP_SKIP_TURN:
        return Message::fromJsonImpl<SkipTurnResponseMessage>(json);

      // Server broadcast messages
      case MessageType::BRDC_PLAYER_LIST:
        return Message::fromJsonImpl<PlayerListUpdateMessage>(json);
      case MessageType::BRDC_GAME_START:
        return Message::fromJsonImpl<GameStartMessage>(json);
      case MessageType::BRDC_GAMESTATE_UPDATE:
        return Message::fromJsonImpl<GameStateUpdateMessage>(json);
      case MessageType::BRDC_PLAYER_DISCONNECTED:
        return Message::fromJsonImpl<PlayerDisconnectedMessage>(json);
      case MessageType::BRDC_PLAYER_FINISHED:
        return Message::fromJsonImpl<PlayerFinishedMessage>(json);
      case MessageType::BRDC_RESULTS:
        return Message::fromJsonImpl<GameResultsMessage>(json);

      // Server private messages
      case MessageType::PRIV_CARDS_DEALT:
        return Message::fromJsonImpl<CardsDealtMessage>(json);
    }

    // This should never be reached - all enum values are handled above
    std::cerr
        << "FATAL ERROR: Message::fromJson() switch reached unreachable code! "
        << "Unhandled MessageType enum value: " << static_cast<int>(messageType)
        << std::endl;
    std::abort();  // Crash immediately with message

  } catch (const std::exception& e) {
    std::cerr << "Error parsing message JSON: " << e.what() << std::endl;
    std::abort();
  }
}
