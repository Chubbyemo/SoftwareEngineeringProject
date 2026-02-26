#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "shared/game.hpp"
#include "shared/game_objects.hpp"
#include "shared/game_types.hpp"

using namespace BraendiDog;

// Test GameState creation and Getters
TEST(GameStateTest, CreationAndGetters) {
  std::array<std::optional<std::string>, 4> playerNames = {"ID0", "ID1", "ID2",
                                                           std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  EXPECT_EQ(gameState.getPlayers().size(), 4);
  EXPECT_EQ(gameState.getCurrentPlayer(), 0);
  EXPECT_EQ(gameState.getRoundStartPlayer(), 0);
  EXPECT_EQ(gameState.getRoundCardCount(), 6);
  EXPECT_EQ(gameState.getLastPlayedCard(), std::nullopt);
  auto leaderBoardExpected = std::array<std::optional<int>, 4>{
      std::nullopt, std::nullopt, std::nullopt, std::nullopt};
  EXPECT_EQ(gameState.getLeaderBoard(), leaderBoardExpected);
  EXPECT_EQ(gameState.getDeck().size(), 54);

  EXPECT_EQ(gameState.getActiveInGameCount(), 3);
  EXPECT_EQ(gameState.getActiveInRoundCount(), 3);
  auto activeIndices = gameState.getActivePlayerIndices();
  EXPECT_EQ(activeIndices.size(), 3);
  EXPECT_EQ(activeIndices, std::vector<size_t>({0, 1, 2}));
}

TEST(GameStateTest, DealCards) {
  std::array<std::optional<std::string>, 4> playerNames = {"ID0", "ID1", "ID2",
                                                           "ID3"};
  BraendiDog::GameState gameState(playerNames);

  auto dealtCards = gameState.dealCards();
  EXPECT_EQ(dealtCards.size(), playerNames.size());  // 4 active players

  for (const auto& [playerIndex, cards] : dealtCards) {
    EXPECT_EQ(cards.size(), 6);  // Each player should get 6 cards
  }

  std::set<size_t> allDealtCardIDs;
  for (const auto& [playerIndex, cards] : dealtCards) {
    allDealtCardIDs.insert(cards.begin(), cards.end());
  }
  EXPECT_EQ(
      allDealtCardIDs.size(),
      playerNames.size() *
          gameState.getRoundCardCount());  // All dealt cards should be unique

  auto dealtCards2ndV = gameState.dealCards();
  EXPECT_EQ(dealtCards2ndV.size(), playerNames.size());  // 4 active players
  // Check if one player's entire hand is different
  for (size_t playerIndex = 0; playerIndex < playerNames.size();
       ++playerIndex) {
    EXPECT_NE(dealtCards[playerIndex], dealtCards2ndV[playerIndex]);
  }
}

TEST(GameStateTest, DeckComposition) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", std::nullopt, std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  const auto& deck = gameState.getDeck();
  EXPECT_EQ(deck.size(), 54);  // Standard Brändi Dog deck size

  std::map<BraendiDog::Rank, size_t> cardCount;
  for (const auto& card : deck) {
    cardCount[card.getRank()]++;
  }

  // Check for standard ranks
  for (int r = static_cast<int>(Rank::ACE); r <= static_cast<int>(Rank::JOKER);
       ++r) {
    if (static_cast<Rank>(r) == BraendiDog::Rank::JOKER) {
      EXPECT_EQ(cardCount[Rank::JOKER], 2) << "Jokers missing or duplicated";
    } else {
      EXPECT_EQ(cardCount[static_cast<Rank>(r)], 4)
          << "Incorrect count for rank " << static_cast<int>(r);
    }
  }
}

TEST(GameStateTest, JsonSerialization) {
  std::array<std::optional<std::string>, 4> playerNames = {"ID0", std::nullopt,
                                                           "ID2", std::nullopt};
  BraendiDog::GameState originalGameState(playerNames);

  // Serialize to JSON
  nlohmann::json json = originalGameState;

  // Deserialize back
  GameState restoredGameState = json.get<GameState>();

  // Check equality
  EXPECT_EQ(restoredGameState.getPlayers().size(),
            originalGameState.getPlayers().size());
  EXPECT_EQ(restoredGameState.getCurrentPlayer(),
            originalGameState.getCurrentPlayer());
  EXPECT_EQ(restoredGameState.getRoundStartPlayer(),
            originalGameState.getRoundStartPlayer());
  EXPECT_EQ(restoredGameState.getRoundCardCount(),
            originalGameState.getRoundCardCount());
  EXPECT_EQ(restoredGameState.getLastPlayedCard(),
            originalGameState.getLastPlayedCard());
  EXPECT_EQ(restoredGameState.getLeaderBoard(),
            originalGameState.getLeaderBoard());
  EXPECT_EQ(restoredGameState.getDeck(), originalGameState.getDeck());
  EXPECT_EQ(restoredGameState.getActiveInGameCount(),
            originalGameState.getActiveInGameCount());
  EXPECT_EQ(restoredGameState.getActiveInRoundCount(),
            originalGameState.getActiveInRoundCount());
  EXPECT_EQ(restoredGameState.getActivePlayerIndices(),
            originalGameState.getActivePlayerIndices());
}

TEST(MoveComputation, isFieldOccupied) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position pos1(BoardLocation::TRACK, 5, 0);
  Position pos2(BoardLocation::TRACK, 10, 1);

  // Manually set some marble positions
  gameState.getPlayers()[0]->setMarblePosition(0, pos1);  // Player 0, Marble 0
  gameState.getPlayers()[1]->setMarblePosition(0, pos2);  // Player 1, Marble 0

  // Check occupied positions
  auto occupiedMarbleOpt = gameState.isFieldOccupied(pos1);
  ASSERT_TRUE(occupiedMarbleOpt.has_value());
  EXPECT_EQ(occupiedMarbleOpt->playerID, 0);
  EXPECT_EQ(occupiedMarbleOpt->marbleIdx, 0);

  occupiedMarbleOpt = gameState.isFieldOccupied(pos2);
  ASSERT_TRUE(occupiedMarbleOpt.has_value());
  EXPECT_EQ(occupiedMarbleOpt->playerID, 1);
  EXPECT_EQ(occupiedMarbleOpt->marbleIdx, 0);

  // Check unoccupied position
  occupiedMarbleOpt =
      gameState.isFieldOccupied(Position(BoardLocation::TRACK, 15, 2));
  EXPECT_FALSE(occupiedMarbleOpt.has_value());
}

TEST(MoveComputation, validateSimpleMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position startPos(BoardLocation::TRACK, 5, 0);
  gameState.getPlayers()[0]->setMarblePosition(0,
                                               startPos);  // Player 0, Marble 0

  // Test moving to unoccupied position
  auto moveResult = gameState.validateMove(Card(Rank::FIVE, Suit::HEARTS),
                                           startPos, {MoveType::SIMPLE, 5});
  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->size(), 1);
  EXPECT_EQ(moveResult->at(0).second, Position(BoardLocation::TRACK, 10, 0));

  // Test moving to occupied position by own marble
  gameState.getPlayers()[0]->setMarblePosition(
      1,
      Position(BoardLocation::TRACK, 10,
               0));  // Occupy target pos with own marble (Player 0, Marble 1)
  moveResult = gameState.validateMove(Card(Rank::FIVE, Suit::HEARTS), startPos,
                                      {MoveType::SIMPLE, 5});
  EXPECT_FALSE(moveResult.has_value());

  // Test moving to occupied position by opponent marble
  gameState.getPlayers()[0]->setMarblePosition(
      1, Position(BoardLocation::HOME, 1,
                  0));  // Move Marble 1 away from the board (back to home)
  gameState.getPlayers()[1]->setMarblePosition(
      0, Position(BoardLocation::TRACK, 10, 0));  // Opponent occupies target
  moveResult = gameState.validateMove(Card(Rank::FIVE, Suit::HEARTS), startPos,
                                      {MoveType::SIMPLE, 5});
  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->size(), 2);
  EXPECT_EQ(moveResult->at(0).second,
            Position(BoardLocation::TRACK, 10, 0));  // Moving marble new pos
  EXPECT_EQ(moveResult->at(1).second,
            Position(BoardLocation::HOME, 0,
                     1));  // Opponent marble sent home (marbleIdx=0)
}

TEST(MoveComputation, validateStartMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position homePos(BoardLocation::HOME, 0, 0);
  Position startFieldPos(BoardLocation::TRACK,
                         gameState.getPlayers()[0]->getStartField(),
                         0);  // Player 0 start field

  // Test moving from home to unoccupied start field
  auto moveResult = gameState.validateMove(Card(Rank::ACE, Suit::SPADES),
                                           homePos, {MoveType::START, 0});
  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->size(), 1);
  EXPECT_EQ(moveResult->at(0).second, startFieldPos);

  // Test moving from home to occupied start field by own marble
  gameState.getPlayers()[0]->setMarblePosition(
      1, startFieldPos);  // Occupy start field with own marble (Player 0,
                          // Marble 1)
  moveResult = gameState.validateMove(
      Card(Rank::ACE, Suit::SPADES), homePos,
      {MoveType::START, 0});  // Check move of Marble 0 to start field
  EXPECT_FALSE(moveResult.has_value());

  // Test moving from home to occupied start field by opponent marble
  gameState.getPlayers()[0]->setMarblePosition(
      1, Position(BoardLocation::HOME, 1,
                  0));  // Move own marble back home (freeing start field)
  gameState.getPlayers()[1]->setMarblePosition(
      0, startFieldPos);  // Opponent occupies start field (Player 1, Marble 0)
  moveResult = gameState.validateMove(
      Card(Rank::ACE, Suit::SPADES), homePos,
      {MoveType::START, 0});  // Check move of Player 0 Marble 0 to start field
  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->size(), 2);
  EXPECT_EQ(moveResult->at(0).second,
            startFieldPos);                        // Moving marble new pos
  EXPECT_EQ(moveResult->at(1).second,              // Opponent marble sent home
            Position(BoardLocation::HOME, 0, 1));  // marbleIdx=0
}

TEST(MoveComputation, validateSwapMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position player0Pos(BoardLocation::TRACK, 20, 0);
  Position player1Pos(BoardLocation::TRACK, 30, 1);

  // Set marble positions
  gameState.getPlayers()[0]->setMarblePosition(
      0,
      player0Pos);  // Player 0, Marble 0
  gameState.getPlayers()[1]->setMarblePosition(
      0,
      player1Pos);  // Player 1, Marble 0

  // Test valid swap move
  auto moveResult = gameState.validateMove(Card(Rank::JACK, Suit::DIAMONDS),
                                           player0Pos, {MoveType::SWAP, 0});
  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->size(), 2);
  EXPECT_EQ(moveResult->at(0).second,
            player1Pos);  // Player 0 marble moves to Player 1 position
  EXPECT_EQ(moveResult->at(1).second,
            player0Pos);  // Player 1 marble moves to Player 0 position

  // Test invalid swap move (no opponent marbles on track)
  gameState.getPlayers()[1]->setMarblePosition(
      0, Position(BoardLocation::HOME, 0,
                  1));  // Move opponent marble back home (no valid swap target)
  moveResult = gameState.validateMove(Card(Rank::JACK, Suit::DIAMONDS),
                                      player0Pos, {MoveType::SWAP, 0});
  EXPECT_FALSE(moveResult.has_value());
}

TEST(MoveComputation, computeLegalMoves) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Setup Player 0
  auto& player0_0 = gameState.getPlayers()[0];
  player0_0->setMarblePosition(0, Position(BoardLocation::TRACK, 5, 0));
  player0_0->setHand(
      {0, 12, 11});  // ACE of CLUBS, KING of CLUBS, QUEEN of CLUBS

  // Compute legal moves for Player 0
  auto legalMoves = gameState.computeLegalMoves();

  // Expecting the following moves:
  // - Move Marble 0 by ACE (1 step)
  // - Move Marble 0 by ACE (11 step)
  // - Move Marble 0 by KING (13 steps)
  // - Move Marble 0 by QUEEN (12 steps)
  // - Move Marble 1 from HOME to start field (ACE)
  // - Move Marble 1 from HOME to start field (KING)
  EXPECT_GE(legalMoves.size(), 6);

  // Setup Player 0 Marble 1 on track to test invalid start move
  auto& player0_1 = gameState.getPlayers()[0];
  player0_1->setMarblePosition(1, Position(BoardLocation::TRACK, 0, 0));

  // Recompute legal moves for Player 0 with Player 1 marble on track
  legalMoves = gameState.computeLegalMoves();

  // Expecting the following moves:
  // - Move Marble 0 by ACE (1 step)
  // - Move Marble 0 by ACE (11 step)
  // - Move Marble 0 by KING (13 steps)
  // - Move Marble 0 by QUEEN (12 steps)
  // - Marble 2 from HOME can not move to start field (occupied by own marble)
  EXPECT_GE(legalMoves.size(), 4);

  // Setup Player 0
  player0_1->setMarblePosition(
      1, Position(BoardLocation::TRACK, 40, 0));  // Move to track location
  // Setup Player 1 Marble 0 on track to test invalid start move
  auto& player1_0 = gameState.getPlayers()[1];
  player1_0->setMarblePosition(0, Position(BoardLocation::TRACK, 20, 1));

  // Setup Player 0 - different hand
  player0_0->setHand({10});  // JACK of CLUBS

  // Recompute legal moves for Player 0 with Player 1 marble on track
  legalMoves = gameState.computeLegalMoves();

  // Expecting the following moves:
  // - Swap Marble 0 with Player 1 Marble 0
  // - Swap Marble 1 with Player 1 Marble 0
  EXPECT_GE(legalMoves.size(), 2);
}

TEST(ServerValidation, ValidFold) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Setup: Player 0 has no legal moves
  auto& player0 = gameState.getPlayers()[0];
  // All marbles in HOME, hand has only cards that can't be used
  player0->setHand({4, 5});  // FIVE and SIX - can't start or move from HOME

  // Validate empty move (fold)
  EXPECT_TRUE(gameState.isValidTurn());
}

TEST(ServerValidation, InvalidFoldWithLegalMoves) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Setup: Player 0 has legal moves
  auto& player0 = gameState.getPlayers()[0];
  player0->setHand({0});  // ACE - can start from HOME

  // Trying to fold when legal moves exist should be invalid
  EXPECT_FALSE(gameState.isValidTurn());
}

TEST(ServerValidation, ValidSimpleMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Setup
  Position startPos(BoardLocation::TRACK, 5, 0);
  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card, cardID=4

  // Create valid move
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, Position(BoardLocation::TRACK, 10, 0)}};
  Move validMove(4, 0, movements);  // cardID=4, handIndex=0

  EXPECT_TRUE(gameState.isValidTurn(validMove));
}

TEST(ServerValidation, InvalidMoveWrongEndPosition) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Setup
  Position startPos(BoardLocation::TRACK, 5, 0);
  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card

  // Create invalid move (wrong distance)
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, Position(BoardLocation::TRACK, 12, 0)}  // Should be 10, not 12
  };
  Move invalidMove(4, 0, movements);

  EXPECT_FALSE(gameState.isValidTurn(invalidMove));
}

TEST(ServerValidation, ValidStartMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position homePos(BoardLocation::HOME, 0, 0);
  Position startFieldPos(BoardLocation::TRACK,
                         gameState.getPlayers()[0]->getStartField(), 0);

  gameState.getPlayers()[0]->setHand({0});  // ACE card, cardID=0

  // Create valid start move
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, startFieldPos}};
  Move validMove(0, 0, movements);

  EXPECT_TRUE(gameState.isValidTurn(validMove));
}

TEST(ServerValidation, ValidMoveWithKickout) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position player0Pos(BoardLocation::TRACK, 5, 0);
  Position targetPos(BoardLocation::TRACK, 10, 0);
  Position opponentHomePos(BoardLocation::HOME, 0, 1);

  // Setup: Player 0 marble at pos 5, Player 1 marble at target pos 10
  gameState.getPlayers()[0]->setMarblePosition(0, player0Pos);
  gameState.getPlayers()[1]->setMarblePosition(0, targetPos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card

  // Create move with kickout (2 movements: active marble + kicked marble)
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, targetPos},       // Player 0 marble moves to target
      {{1, 0}, opponentHomePos}  // Player 1 marble sent home
  };
  Move validMove(4, 0, movements);

  EXPECT_TRUE(gameState.isValidTurn(validMove));
}

TEST(ServerValidation, InvalidMoveWrongMarble) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position player0Marble0Pos(BoardLocation::TRACK, 5, 0);
  Position player0Marble1Pos(BoardLocation::TRACK, 15, 0);

  gameState.getPlayers()[0]->setMarblePosition(0, player0Marble0Pos);
  gameState.getPlayers()[0]->setMarblePosition(1, player0Marble1Pos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card

  // Try to move marble 0 but claim marble 1 moved
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 1}, Position(BoardLocation::TRACK, 10, 0)}  // Wrong marble ID
  };
  Move invalidMove(4, 0, movements);

  EXPECT_FALSE(gameState.isValidTurn(invalidMove));
}

TEST(ServerValidation, ValidSwapMove) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position player0Pos(BoardLocation::TRACK, 20, 0);
  Position player1Pos(BoardLocation::TRACK, 30, 1);

  gameState.getPlayers()[0]->setMarblePosition(0, player0Pos);
  gameState.getPlayers()[1]->setMarblePosition(0, player1Pos);
  gameState.getPlayers()[0]->setHand({10});  // JACK card, cardID=10

  // Create valid swap move (both marbles move)
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, player1Pos},  // Player 0 marble to player 1 position
      {{1, 0}, player0Pos}   // Player 1 marble to player 0 position
  };
  Move validMove(10, 0, movements);

  EXPECT_TRUE(gameState.isValidTurn(validMove));
}

TEST(MoveLogic, StartFieldBlocking) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position homePos(BoardLocation::HOME, 0, 0);
  Position startFieldPos(BoardLocation::TRACK,
                         gameState.getPlayers()[0]->getStartField(), 0);

  // Move marble 0 from HOME to start field
  gameState.getPlayers()[0]->setHand({0});  // ACE card
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, startFieldPos}};
  Move startMove(0, 0, movements);

  EXPECT_TRUE(gameState.isValidTurn(startMove));
  gameState.executeMove(startMove);

  // Verify start field is now blocked for marble 0
  EXPECT_TRUE(gameState.getPlayers()[0]->isStartBlocked());
  EXPECT_EQ(gameState.getPlayers()[0]->getStartBlocked().value(), 0);

  // Now try to move marble 0 forward - it should NOT be able to enter finish
  // zone
  size_t startField = gameState.getPlayers()[0]->getStartField();
  gameState.getPlayers()[0]->setHand({2});  // THREE card

  // This move would normally enter finish (3 steps from start field)
  // But since start is blocked, it should wrap around track instead
  auto moveResult = gameState.validateMove(
      Card(Rank::THREE, Suit::HEARTS), startFieldPos, {MoveType::SIMPLE, 3});

  ASSERT_TRUE(moveResult.has_value());
  // Should stay on TRACK, not enter FINISH
  EXPECT_EQ(moveResult->at(0).second.boardLocation, BoardLocation::TRACK);
  EXPECT_EQ(moveResult->at(0).second.index, (startField + 3) % 64);
}

TEST(MoveLogic, StartFieldUnblocking) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position startFieldPos(BoardLocation::TRACK,
                         gameState.getPlayers()[0]->getStartField(), 0);

  // Set up: marble at start field with start blocked
  gameState.getPlayers()[0]->setMarblePosition(0, startFieldPos);
  gameState.getPlayers()[0]->setStartBlocked(0);

  EXPECT_TRUE(gameState.getPlayers()[0]->isStartBlocked());

  // Move the blocking marble away
  gameState.getPlayers()[0]->setHand({4});  // FIVE card
  Position newPos(BoardLocation::TRACK, startFieldPos.index + 5, 0);
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, newPos}};
  Move unblockMove(4, 0, movements);

  gameState.executeMove(unblockMove);

  // Verify start field is now unblocked
  EXPECT_FALSE(gameState.getPlayers()[0]->isStartBlocked());
}

TEST(MoveLogic, CannotCrossBlockedStartField) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  size_t player0StartField = gameState.getPlayers()[0]->getStartField();
  size_t player1StartField = gameState.getPlayers()[1]->getStartField();

  // Player 1 has marble on their start field (blocked)
  Position player1StartPos(BoardLocation::TRACK, player1StartField, 1);
  gameState.getPlayers()[1]->setMarblePosition(0, player1StartPos);
  gameState.getPlayers()[1]->setStartBlocked(0);

  // Player 0 tries to move across Player 1's blocked start field
  Position player0Pos(BoardLocation::TRACK, player1StartField - 3, 0);
  gameState.getPlayers()[0]->setMarblePosition(0, player0Pos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card (would cross)

  auto moveResult = gameState.validateMove(Card(Rank::FIVE, Suit::HEARTS),
                                           player0Pos, {MoveType::SIMPLE, 5});

  EXPECT_FALSE(moveResult.has_value());  // Move should be blocked
}

TEST(MoveLogic, MovingWithinFinishZone) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Marble already in finish zone at position 0
  Position finishPos(BoardLocation::FINISH, 0, 0);  // Changed from 1 to 0
  gameState.getPlayers()[0]->setMarblePosition(0, finishPos);
  gameState.getPlayers()[0]->setHand({2});  // THREE card

  // Move within finish zone - 3 steps from position 0 = position 3
  auto moveResult = gameState.validateMove(Card(Rank::THREE, Suit::DIAMONDS),
                                           finishPos, {MoveType::SIMPLE, 3});

  ASSERT_TRUE(moveResult.has_value());
  EXPECT_EQ(moveResult->at(0).second.boardLocation, BoardLocation::FINISH);
  EXPECT_EQ(moveResult->at(0).second.index,
            3);  // 0 + 3 = 3 (valid, max position!)
}

TEST(MoveLogic, CannotExceedFinishZone) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Marble at finish position 3
  Position finishPos(BoardLocation::FINISH, 3, 0);
  gameState.getPlayers()[0]->setMarblePosition(0, finishPos);
  gameState.getPlayers()[0]->setHand({2});  // THREE card (would exceed)

  // Try to move 3 steps (would go to position 6, which exceeds max 4)
  auto moveResult = gameState.validateMove(Card(Rank::THREE, Suit::DIAMONDS),
                                           finishPos, {MoveType::SIMPLE, 3});

  EXPECT_FALSE(moveResult.has_value());  // Should be invalid
}

TEST(MoveLogic, BlockedStartPreventesFinishEntry) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  size_t startField = gameState.getPlayers()[0]->getStartField();

  // Marble on start field with start blocked
  Position startPos(BoardLocation::TRACK, startField, 0);
  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setStartBlocked(0);
  gameState.getPlayers()[0]->setHand({2});  // THREE card

  // Try to move - should stay on track, not enter finish
  auto moveResult = gameState.validateMove(Card(Rank::THREE, Suit::HEARTS),
                                           startPos, {MoveType::SIMPLE, 3});

  ASSERT_TRUE(moveResult.has_value());
  // Should wrap around track, not enter finish
  EXPECT_EQ(moveResult->at(0).second.boardLocation, BoardLocation::TRACK);
  EXPECT_EQ(moveResult->at(0).second.index, (startField + 3) % 64);
}

TEST(ExecuteMove, MarblePositionUpdate) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position startPos(BoardLocation::TRACK, 5, 0);
  Position endPos(BoardLocation::TRACK, 10, 0);

  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setHand({4});  // FIVE card

  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, endPos}};
  Move move(4, 0, movements);

  gameState.executeMove(move);

  // Verify marble moved
  EXPECT_EQ(gameState.getPlayers()[0]->getMarblePosition(0), endPos);
}

TEST(ExecuteMove, CardRemovedFromHand) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position startPos(BoardLocation::TRACK, 5, 0);
  Position endPos(BoardLocation::TRACK, 10, 0);

  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setHand({4, 5, 6});  // Three cards

  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, endPos}};
  Move move(4, 0, movements);  // Play first card (handIndex=0)

  gameState.executeMove(move);

  // Verify card removed and hand size decreased
  auto hand = gameState.getPlayers()[0]->getHand();
  EXPECT_EQ(hand.size(), 2);
  EXPECT_EQ(hand[0], 5);  // Original index 1
  EXPECT_EQ(hand[1], 6);  // Original index 2
}

TEST(ExecuteMove, LastPlayedCardUpdated) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  Position startPos(BoardLocation::TRACK, 5, 0);
  Position endPos(BoardLocation::TRACK, 10, 0);

  gameState.getPlayers()[0]->setMarblePosition(0, startPos);
  gameState.getPlayers()[0]->setHand({4});

  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 0}, endPos}};
  Move move(4, 0, movements);

  gameState.executeMove(move);

  EXPECT_EQ(gameState.getLastPlayedCard().value(), 4);
}

TEST(ExecuteMove, PlayerFinishDetection) {
  std::array<std::optional<std::string>, 4> playerNames = {
      "ID0", "ID1", std::nullopt, std::nullopt};
  BraendiDog::GameState gameState(playerNames);

  // Place all marbles in finish zone except one - use valid indices (0-3)
  gameState.getPlayers()[0]->setMarblePosition(
      0, Position(BoardLocation::FINISH, 3, 0));  // Changed from 4 to 3
  gameState.getPlayers()[0]->setMarblePosition(
      1, Position(BoardLocation::FINISH, 3, 0));  // Changed from 4 to 3
  gameState.getPlayers()[0]->setMarblePosition(
      2, Position(BoardLocation::FINISH, 3, 0));  // Changed from 4 to 3
  gameState.getPlayers()[0]->setMarblePosition(
      3, Position(BoardLocation::FINISH, 2, 0));  // Changed from 3 to 2

  gameState.getPlayers()[0]->setHand({0});  // ACE card

  // Move last marble to finish
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {{0, 3}, Position(BoardLocation::FINISH, 3, 0)}  // Changed from 4 to 3
  };
  Move move(0, 0, movements);

  gameState.executeMove(move);

  // Verify player is marked as finished
  EXPECT_FALSE(gameState.getPlayers()[0]->isActiveInGame());
  EXPECT_EQ(gameState.getLeaderBoard()[0], 1);  // First finisher
}