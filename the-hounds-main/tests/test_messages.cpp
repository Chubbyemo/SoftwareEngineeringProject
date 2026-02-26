// #include <gtest/gtest.h>

// #include <nlohmann/json.hpp>

// #include "shared/messages.hpp"

// class MessageTest : public ::testing::Test {};

// //
// -----------------------------------------------------------------------------
// // CLIENT → SERVER MESSAGES
// //
// -----------------------------------------------------------------------------

// TEST_F(MessageTest, ConnectionRequestMessage) {
//   ConnectionRequestMessage msg("Sophie");
//   nlohmann::json j = msg.toJson();

//   EXPECT_EQ(j["msgType"], "REQ_CONNECT");
//   EXPECT_EQ(j["name"], "Sophie");

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_CONNECT);
//   auto* m = dynamic_cast<ConnectionRequestMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->name, "Sophie");
// }

// TEST_F(MessageTest, ReadyMessage) {
//   ReadyMessage msg = ReadyMessage(3);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "REQ_READY");
//   EXPECT_EQ(j["playerId_"], 3);
//   EXPECT_EQ(j.size(), 2);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_READY);
//   auto* m = dynamic_cast<ReadyMessage*>(parsed.get());
//   EXPECT_EQ(m->getPlayerId(), 3);
// }

// TEST_F(MessageTest, StartGameRequestMessage) {
//   StartGameRequestMessage msg = StartGameRequestMessage(0);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j.size(), 2);
//   EXPECT_EQ(j["msgType"], "REQ_START_GAME");
//   EXPECT_EQ(j["playerId_"], 0);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_START_GAME);
//   auto* m = dynamic_cast<StartGameRequestMessage*>(parsed.get());
//   EXPECT_EQ(m->getPlayerId(), 0);
// }

// // TEST_F(MessageTest, PlayCardRequestMessage) {
// //   PlayCardRequestMessage msg(42);
// //   nlohmann::json j = msg.toJson();
// //   EXPECT_EQ(j["msgType"], "REQ_PLAY_CARD");
// //   EXPECT_EQ(j["card"], 42);

// //   auto parsed = Message::fromJson(j);
// //   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_PLAY_CARD);
// //   auto* m = dynamic_cast<PlayCardRequestMessage*>(parsed.get());
// //   ASSERT_NE(m, nullptr);
// //   EXPECT_EQ(m->card, 42);
// // }

// TEST_F(MessageTest, SkipTurnRequestMessage) {
//   SkipTurnRequestMessage msg = SkipTurnRequestMessage(1);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "REQ_SKIP_TURN");
//   EXPECT_EQ(j["playerId_"], 1);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_SKIP_TURN);
//   auto* m = dynamic_cast<SkipTurnRequestMessage*>(parsed.get());
//   EXPECT_EQ(m->getPlayerId(), 1);
// }

// TEST_F(MessageTest, DisconnectRequestMessage) {
//   DisconnectRequestMessage msg(0);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "REQ_DISCONNECT");
//   EXPECT_EQ(j["playerId_"], 0);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_DISCONNECT);
//   auto* m = dynamic_cast<DisconnectRequestMessage*>(parsed.get());
//   EXPECT_EQ(m->getPlayerId(), 0);
// }

// //
// -----------------------------------------------------------------------------
// // SERVER → CLIENT RESPONSES
// //
// -----------------------------------------------------------------------------

// TEST_F(MessageTest, ConnectionResponseMessage) {
//   ConnectionResponseMessage msg(true, "", 2);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "RESP_CONNECT");
//   EXPECT_EQ(j["playerId"], 2);
//   EXPECT_TRUE(j["success_"]);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_CONNECT);
//   auto* m = dynamic_cast<ConnectionResponseMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_TRUE(m->getSuccess());
//   EXPECT_EQ(m->playerId, 2);
// }

// TEST_F(MessageTest, StartGameResponseMessage) {
//   StartGameResponseMessage msg(true, "ok");
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "RESP_START_GAME");
//   EXPECT_TRUE(j["success_"]);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_START_GAME);
//   auto* m = dynamic_cast<StartGameResponseMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_TRUE(m->getSuccess());
// }

// TEST_F(MessageTest, PlayCardResponseMessage) {
//   PlayCardResponseMessage msg(true, "");
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "RESP_PLAY_CARD");
//   EXPECT_TRUE(j["success_"]);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_PLAY_CARD);
//   auto* m = dynamic_cast<PlayCardResponseMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_TRUE(m->getSuccess());
// }

// TEST_F(MessageTest, SkipTurnResponseMessage) {
//   SkipTurnResponseMessage msg(true, "");
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "RESP_SKIP_TURN");
//   EXPECT_TRUE(j["success_"]);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_SKIP_TURN);
//   EXPECT_NE(dynamic_cast<SkipTurnResponseMessage*>(parsed.get()), nullptr);
// }

// TEST_F(MessageTest, DisconnectResponseMessage) {
//   DisconnectResponseMessage msg(true, "");
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "RESP_DISCONNECT");
//   EXPECT_TRUE(j["success_"]);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_DISCONNECT);
//   auto* m = dynamic_cast<DisconnectResponseMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_TRUE(m->getSuccess());
// }

// //
// -----------------------------------------------------------------------------
// // SERVER BROADCAST MESSAGES
// //
// -----------------------------------------------------------------------------

// TEST_F(MessageTest, PlayerListUpdateMessage) {
//   std::vector<PlayerInfo> list = {{0, "Alice", true}, {2, "Bob", false}};
//   PlayerListUpdateMessage msg(list);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_PLAYER_LIST");
//   EXPECT_EQ(j["playersList"].size(), 2);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_LIST);
//   auto* m = dynamic_cast<PlayerListUpdateMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->playersList.size(), 2);
// }

// TEST_F(MessageTest, GameStartMessage) {
//   GameStartMessage msg(4);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_GAME_START");
//   EXPECT_EQ(j["numPlayers"], 4);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_GAME_START);
//   auto* m = dynamic_cast<GameStartMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->numPlayers, 4);
// }

// // TEST_F(MessageTest, GameStateUpdateMessage) {
// //   GameState state;
// //   // state.turn = 1;  // assume GameState has some fields
// //   GameStateUpdateMessage msg(state);
// //   nlohmann::json j = msg.toJson();
// //   EXPECT_EQ(j["msgType"], "BRDC_GAMESTATE_UPDATE");

// //   auto parsed = Message::fromJson(j);
// //   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_GAMESTATE_UPDATE);
// //   EXPECT_NE(dynamic_cast<GameStateUpdateMessage*>(parsed.get()), nullptr);
// // }

// TEST_F(MessageTest, PlayerDisconnectedMessage) {
//   PlayerDisconnectedMessage msg(1);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_PLAYER_DISCONNECTED");
//   EXPECT_EQ(j["playerId"], 1);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_DISCONNECTED);
//   auto* m = dynamic_cast<PlayerDisconnectedMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->playerId, 1);
// }

// // TEST_F(MessageTest, PlayerFinishedMessage) {
// //   PlayerFinishedMessage msg(1);
// //   nlohmann::json j = msg.toJson();
// //   EXPECT_EQ(j["msgType"], "BRDC_PLAYER_FINISHED");
// //   EXPECT_EQ(j["playerName"], "Bob");
// //   EXPECT_EQ(j["rank"], 1);

// //   auto parsed = Message::fromJson(j);
// //   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_FINISHED);
// //   auto* m = dynamic_cast<PlayerFinishedMessage*>(parsed.get());
// //   ASSERT_NE(m, nullptr);
// //   EXPECT_EQ(m->rank, 1);
// // }

// // TEST_F(MessageTest, GameResultsMessage) {
// //   std::vector<std::string> winners = {"Alice", "Charlie"};
// //   GameResultsMessage msg(winners);
// //   nlohmann::json j = msg.toJson();
// //   EXPECT_EQ(j["msgType"], "BRDC_RESULTS");
// //   EXPECT_EQ(j["winners"].size(), 2);

// //   auto parsed = Message::fromJson(j);
// //   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_RESULTS);
// //   auto* m = dynamic_cast<GameResultsMessage*>(parsed.get());
// //   ASSERT_NE(m, nullptr);
// //   EXPECT_EQ(m->winners.size(), 2);
// // }

// //
// -----------------------------------------------------------------------------
// // SERVER PRIVATE MESSAGES
// //
// -----------------------------------------------------------------------------

// // TEST_F(MessageTest, CardsDealtMessage) {
// //   std::vector<int> cards = {1, 7, 13};
// //   CardsDealtMessage msg(0, cards);
// //   nlohmann::json j = msg.toJson();
// //   EXPECT_EQ(j["msgType"], "PRIV_CARDS_DEALT");
// //   EXPECT_EQ(j["cards"].size(), 3);

// //   auto parsed = Message::fromJson(j);
// //   EXPECT_EQ(parsed->getMessageType(), MessageType::PRIV_CARDS_DEALT);
// //   auto* m = dynamic_cast<CardsDealtMessage*>(parsed.get());
// //   ASSERT_NE(m, nullptr);
// //   EXPECT_EQ(m->cards.size(), 3);
// // }

// //
// -----------------------------------------------------------------------------
// // META MESSAGES
// //
// -----------------------------------------------------------------------------

// TEST_F(MessageTest, ErrorMessage) {
//   ErrorMessage msg(422, "Invalid move");
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "ERROR");
//   EXPECT_EQ(j["errorCode"], 422);
//   EXPECT_EQ(j["errorMsg"], "Invalid move");

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::ERROR);
//   auto* m = dynamic_cast<ErrorMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->errorCode, 422);
//   EXPECT_EQ(m->errorMsg, "Invalid move");
// }

// TEST_F(MessageTest, HeartbeatMessage) {
//   auto now = std::chrono::steady_clock::now();
//   size_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(
//                   now.time_since_epoch())
//                   .count();
//   HeartbeatMessage msg = HeartbeatMessage(1, ts);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "HEARTBEAT");
//   EXPECT_EQ(j["playerId"], 1);
//   EXPECT_EQ(j["timestamp"], ts);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::HEARTBEAT);
//   auto* m = dynamic_cast<HeartbeatMessage*>(parsed.get());
//   EXPECT_EQ(m->playerId, 1);
//   EXPECT_EQ(m->timestamp, ts);
// }
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "shared/messages.hpp"

class MessageTest : public ::testing::Test {};

// -----------------------------------------------------------------------------
// CLIENT → SERVER MESSAGES
// -----------------------------------------------------------------------------

TEST_F(MessageTest, ConnectionRequestMessage) {
  ConnectionRequestMessage msg("Sophie");
  nlohmann::json j = msg.toJson();

  EXPECT_EQ(j["msgType"], "REQ_CONNECT");
  EXPECT_EQ(j["name"], "Sophie");

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_CONNECT);
  auto* m = dynamic_cast<ConnectionRequestMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_EQ(m->name, "Sophie");
}

TEST_F(MessageTest, ReadyMessage) {
  ReadyMessage msg = ReadyMessage(3);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "REQ_READY");
  EXPECT_EQ(j["playerId_"], 3);
  EXPECT_EQ(j.size(), 2);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_READY);
  auto* m = dynamic_cast<ReadyMessage*>(parsed.get());
  EXPECT_EQ(m->getPlayerId(), 3);
}

TEST_F(MessageTest, StartGameRequestMessage) {
  StartGameRequestMessage msg = StartGameRequestMessage(0);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j.size(), 2);
  EXPECT_EQ(j["msgType"], "REQ_START_GAME");
  EXPECT_EQ(j["playerId_"], 0);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_START_GAME);
  auto* m = dynamic_cast<StartGameRequestMessage*>(parsed.get());
  EXPECT_EQ(m->getPlayerId(), 0);
}

// TEST_F(MessageTest, PlayCardRequestMessage) {
//   PlayCardRequestMessage msg(42);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "REQ_PLAY_CARD");
//   EXPECT_EQ(j["card"], 42);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_PLAY_CARD);
//   auto* m = dynamic_cast<PlayCardRequestMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->card, 42);
// }

TEST_F(MessageTest, SkipTurnRequestMessage) {
  SkipTurnRequestMessage msg = SkipTurnRequestMessage(1);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "REQ_SKIP_TURN");
  EXPECT_EQ(j["playerId_"], 1);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::REQ_SKIP_TURN);
  auto* m = dynamic_cast<SkipTurnRequestMessage*>(parsed.get());
  EXPECT_EQ(m->getPlayerId(), 1);
}

// -----------------------------------------------------------------------------
// SERVER → CLIENT RESPONSES
// -----------------------------------------------------------------------------

TEST_F(MessageTest, ConnectionResponseMessage) {
  ConnectionResponseMessage msg(true, "", 2);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "RESP_CONNECT");
  EXPECT_EQ(j["playerId"], 2);
  EXPECT_TRUE(j["success_"]);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_CONNECT);
  auto* m = dynamic_cast<ConnectionResponseMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_TRUE(m->getSuccess());
  EXPECT_EQ(m->playerId, 2);
}

TEST_F(MessageTest, StartGameResponseMessage) {
  StartGameResponseMessage msg(true, "ok");
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "RESP_START_GAME");
  EXPECT_TRUE(j["success_"]);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_START_GAME);
  auto* m = dynamic_cast<StartGameResponseMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_TRUE(m->getSuccess());
}

TEST_F(MessageTest, PlayCardResponseMessage) {
  PlayCardResponseMessage msg(true, "");
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "RESP_PLAY_CARD");
  EXPECT_TRUE(j["success_"]);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_PLAY_CARD);
  auto* m = dynamic_cast<PlayCardResponseMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_TRUE(m->getSuccess());
}

TEST_F(MessageTest, SkipTurnResponseMessage) {
  SkipTurnResponseMessage msg(true, "");
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "RESP_SKIP_TURN");
  EXPECT_TRUE(j["success_"]);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::RESP_SKIP_TURN);
  EXPECT_NE(dynamic_cast<SkipTurnResponseMessage*>(parsed.get()), nullptr);
}

// -----------------------------------------------------------------------------
// SERVER BROADCAST MESSAGES
// -----------------------------------------------------------------------------

TEST_F(MessageTest, PlayerListUpdateMessage) {
  std::vector<PlayerInfo> list = {{0, "Alice", true}, {2, "Bob", false}};
  PlayerListUpdateMessage msg(list);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "BRDC_PLAYER_LIST");
  EXPECT_EQ(j["playersList"].size(), 2);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_LIST);
  auto* m = dynamic_cast<PlayerListUpdateMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_EQ(m->playersList.size(), 2);
}

TEST_F(MessageTest, GameStartMessage) {
  GameStartMessage msg(4);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "BRDC_GAME_START");
  EXPECT_EQ(j["numPlayers"], 4);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_GAME_START);
  auto* m = dynamic_cast<GameStartMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_EQ(m->numPlayers, 4);
}

// TEST_F(MessageTest, GameStateUpdateMessage) {
//   GameState state;
//   // state.turn = 1;  // assume GameState has some fields
//   GameStateUpdateMessage msg(state);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_GAMESTATE_UPDATE");

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_GAMESTATE_UPDATE);
//   EXPECT_NE(dynamic_cast<GameStateUpdateMessage*>(parsed.get()), nullptr);
// }

TEST_F(MessageTest, PlayerDisconnectedMessage) {
  PlayerDisconnectedMessage msg(1);
  nlohmann::json j = msg.toJson();
  EXPECT_EQ(j["msgType"], "BRDC_PLAYER_DISCONNECTED");
  EXPECT_EQ(j["playerId"], 1);

  auto parsed = Message::fromJson(j);
  EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_DISCONNECTED);
  auto* m = dynamic_cast<PlayerDisconnectedMessage*>(parsed.get());
  ASSERT_NE(m, nullptr);
  EXPECT_EQ(m->playerId, 1);
}

// TEST_F(MessageTest, PlayerFinishedMessage) {
//   PlayerFinishedMessage msg(1);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_PLAYER_FINISHED");
//   EXPECT_EQ(j["playerName"], "Bob");
//   EXPECT_EQ(j["rank"], 1);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_PLAYER_FINISHED);
//   auto* m = dynamic_cast<PlayerFinishedMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->rank, 1);
// }

// TEST_F(MessageTest, GameResultsMessage) {
//   std::vector<std::string> winners = {"Alice", "Charlie"};
//   GameResultsMessage msg(winners);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "BRDC_RESULTS");
//   EXPECT_EQ(j["winners"].size(), 2);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::BRDC_RESULTS);
//   auto* m = dynamic_cast<GameResultsMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->winners.size(), 2);
// }

// -----------------------------------------------------------------------------
// SERVER PRIVATE MESSAGES
// -----------------------------------------------------------------------------

// TEST_F(MessageTest, CardsDealtMessage) {
//   std::vector<int> cards = {1, 7, 13};
//   CardsDealtMessage msg(0, cards);
//   nlohmann::json j = msg.toJson();
//   EXPECT_EQ(j["msgType"], "PRIV_CARDS_DEALT");
//   EXPECT_EQ(j["cards"].size(), 3);

//   auto parsed = Message::fromJson(j);
//   EXPECT_EQ(parsed->getMessageType(), MessageType::PRIV_CARDS_DEALT);
//   auto* m = dynamic_cast<CardsDealtMessage*>(parsed.get());
//   ASSERT_NE(m, nullptr);
//   EXPECT_EQ(m->cards.size(), 3);
// }
