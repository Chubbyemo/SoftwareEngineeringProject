// #include <gtest/gtest.h>

// #include <nlohmann/json.hpp>

// #include "shared/game.hpp"
// #include "shared/game_objects.hpp"
// #include "shared/game_types.hpp"

// using namespace BraendiDog;

// // Test Position creation and equality
// TEST(PositionTest, CreateAndCompare) {
//   Position pos1(BoardLocation::TRACK, 5, 0);
//   Position pos2(BoardLocation::TRACK, 5, 1);
//   Position pos3(BoardLocation::HOME, 3, 0);
//   Position pos4(BoardLocation::HOME, 3, 1);
//   Position pos5(BoardLocation::FINISH, 2, 0);

//   EXPECT_TRUE(pos1.equals(pos2));  // Same position on TRACK differs in
//   playerID EXPECT_FALSE(pos1.equals(pos3));  // Different location
//   EXPECT_FALSE(
//       pos3.equals(pos4));  // Different playerID in "same HOME" position
// }

// TEST(PositionTest, InvalidPositionThrows) {
//   // Invalid playerID
//   EXPECT_THROW(Position(BoardLocation::HOME, 0, 4), std::out_of_range);

//   // Invalid index for HOME or FINISH
//   EXPECT_THROW(Position(BoardLocation::HOME, 4, 0), std::out_of_range);
//   EXPECT_THROW(Position(BoardLocation::FINISH, 5, 1), std::out_of_range);

//   // Invalid index for TRACK
//   EXPECT_THROW(Position(BoardLocation::TRACK, 64, 2), std::out_of_range);
// }

// TEST(PositionTest, IsInHomeAndFinish) {
//   Position posH(BoardLocation::HOME, 3, 1);
//   Position posF(BoardLocation::FINISH, 2, 0);
//   Position posT(BoardLocation::TRACK, 10, 2);

//   // Home checks
//   EXPECT_TRUE(posH.isInHome());
//   EXPECT_FALSE(posH.isInFinish());
//   // Finish checks
//   EXPECT_FALSE(posF.isInHome());
//   EXPECT_TRUE(posF.isInFinish());
//   // Track checks
//   EXPECT_FALSE(posT.isInHome());
//   EXPECT_FALSE(posT.isInFinish());
// }

// // Test Position JSON round-trip
// TEST(PositionTest, JsonSerialization) {
//   Position original(BoardLocation::FINISH, 2, 1);

//   // Serialize to JSON
//   nlohmann::json json = original;

//   // Deserialize back
//   Position restored = json.get<Position>();

//   // Should be equal
//   EXPECT_TRUE(original.equals(restored));
//   EXPECT_EQ(restored.index, 2);
//   EXPECT_EQ(restored.playerID, 1);
// }

// // Test MarbleIdentifier Creation and Getters
// TEST(MarbleIdentifierTest, CreateAndGetters) {
//   MarbleIdentifier mid(2, 3);

//   EXPECT_EQ(mid.playerID, 2);
//   EXPECT_EQ(mid.marbleIdx, 3);
// }

// // Test MarbleIdentifier Invalid Creation
// TEST(MarbleIdentifierTest, InvalidCreationThrows) {
//   // Invalid playerID
//   EXPECT_THROW(MarbleIdentifier(4, 0), std::out_of_range);

//   // Invalid marbleIdx
//   EXPECT_THROW(MarbleIdentifier(1, 4), std::out_of_range);
// }

// // Test MarbleIdentifier JSON round-trip
// TEST(MarbleIdentifierTest, JsonSerialization) {
//   MarbleIdentifier original(1, 2);

//   // Serialize to JSON
//   nlohmann::json json = original;

//   // Deserialize back
//   MarbleIdentifier restored = json.get<MarbleIdentifier>();

//   // Check equality
//   EXPECT_EQ(restored.playerID, original.playerID);
//   EXPECT_EQ(restored.marbleIdx, original.marbleIdx);
// }

// // Test Move Creation and Getters
// TEST(MoveTest, CreateAndGetters) {
//   size_t cardID = 20;
//   size_t handIndex = 3;

//   Position posA(BoardLocation::TRACK, 30, 0);
//   Position posB(BoardLocation::HOME, 0, 1);
//   std::vector<std::pair<MarbleIdentifier, Position>> movements = {
//       {MarbleIdentifier(0, 1), posA}, {MarbleIdentifier(1, 2), posB}};
//   Move move(cardID, handIndex, movements);

//   EXPECT_EQ(move.getCardID(), cardID);
//   EXPECT_EQ(move.getHandIndex(), handIndex);
//   EXPECT_EQ(move.getMovements().size(), movements.size());
//   EXPECT_EQ(move.getMovements()[0].first.playerID, 0);
//   EXPECT_EQ(move.getMovements()[0].first.marbleIdx, 1);
//   EXPECT_TRUE(move.getMovements()[0].second.equals(posA));
//   EXPECT_EQ(move.getMovements()[1].first.playerID, 1);
//   EXPECT_EQ(move.getMovements()[1].first.marbleIdx, 2);
//   EXPECT_TRUE(move.getMovements()[1].second.equals(posB));
// }

// // Test Move Invalid Creation
// TEST(MoveTest, InvalidCreationThrows) {
//   size_t handIndex = 2;
//   Position pos(BoardLocation::TRACK, 10, 0);
//   std::vector<std::pair<MarbleIdentifier, Position>> movements = {
//       {MarbleIdentifier(0, 0), pos}};

//   // Invalid cardID
//   EXPECT_THROW(Move(70, handIndex, movements), std::out_of_range);
// }

// // Test Card creation and getters
// TEST(CardTest, CreateAndGetters) {
//   Rank rank = Rank::ACE;
//   Suit suit = Suit::HEARTS;
//   std::vector<std::pair<MoveType, int>> moveRules = {
//       {MoveType::SIMPLE, 1}, {MoveType::SIMPLE, 11}, {MoveType::START, 0}};

//   Card card(rank, suit, moveRules);

//   EXPECT_EQ(card.getRank(), rank);
//   EXPECT_EQ(card.getSuit(), suit);
//   EXPECT_EQ(card.getMoveRules().size(), moveRules.size());
//   EXPECT_EQ(card.getMoveRules()[0].first, MoveType::SIMPLE);
//   EXPECT_EQ(card.getMoveRules()[0].second, 1);
//   EXPECT_EQ(card.getMoveRules()[1].first, MoveType::SIMPLE);
//   EXPECT_EQ(card.getMoveRules()[1].second, 11);
//   EXPECT_EQ(card.getMoveRules()[2].first, MoveType::START);
//   EXPECT_EQ(card.getMoveRules()[2].second, 0);
// }

// // Test Card JSON round-trip
// TEST(CardTest, JsonSerialization) {
//   Rank rank = Rank::KING;
//   Suit suit = Suit::SPADES;
//   std::vector<std::pair<MoveType, int>> moveRules = {{MoveType::SIMPLE, 13},
//                                                      {MoveType::START, 0}};

//   Card originalCard(rank, suit, moveRules);

//   // Serialize to JSON
//   nlohmann::json json = originalCard;

//   // Deserialize back
//   Card restoredCard = json.get<Card>();

//   // Check equality
//   EXPECT_EQ(restoredCard.getRank(), originalCard.getRank());
//   EXPECT_EQ(restoredCard.getSuit(), originalCard.getSuit());
//   EXPECT_EQ(restoredCard.getMoveRules().size(),
//             originalCard.getMoveRules().size());
//   for (size_t i = 0; i < restoredCard.getMoveRules().size(); ++i) {
//     EXPECT_EQ(restoredCard.getMoveRules()[i].first,
//               originalCard.getMoveRules()[i].first);
//     EXPECT_EQ(restoredCard.getMoveRules()[i].second,
//               originalCard.getMoveRules()[i].second);
//   }
// }

// // Test Player creation, getters and setters
// TEST(PlayerTest, CreateAndGettersSetters) {
//   size_t playerID = 1;
//   std::string playerName = "TestPlayer";
//   Player testPlayer(playerID, playerName);

//   // Check initial values
//   EXPECT_EQ(testPlayer.getId(), playerID);
//   EXPECT_EQ(testPlayer.getName(), playerName);
//   EXPECT_EQ(testPlayer.getStartField(), 16 * playerID);
//   EXPECT_EQ(testPlayer.getMarbles().size(), 4);
//   EXPECT_TRUE(testPlayer.isActiveInRound());
//   EXPECT_TRUE(testPlayer.getHand().empty());
//   EXPECT_FALSE(testPlayer.isStartBlocked());  // Initially not blocked

//   // Set active in round
//   testPlayer.setActiveInRound(false);
//   EXPECT_FALSE(testPlayer.isActiveInRound());

//   // Set hand
//   std::vector<size_t> handCards = {3, 7, 11};
//   testPlayer.setHand(handCards);
//   EXPECT_EQ(testPlayer.getHand().size(), handCards.size());
//   for (size_t i = 0; i < handCards.size(); ++i) {
//     EXPECT_EQ(testPlayer.getHand()[i], handCards[i]);
//   }
// }

// // Test Player initial marble positions
// TEST(PlayerTest, InitialMarblePositions) {
//   size_t playerID = 2;
//   Player testPlayer(playerID, "Player2");

//   // All marbles should start in HOME
//   for (size_t i = 0; i < 4; ++i) {
//     Position pos = testPlayer.getMarblePosition(i);
//     EXPECT_TRUE(pos.isInHome());
//     EXPECT_EQ(pos.boardLocation, BoardLocation::HOME);
//     EXPECT_EQ(pos.index, i);
//     EXPECT_EQ(pos.playerID, playerID);
//   }
// }

// // Test Player marble position getters and setters
// TEST(PlayerTest, MarblePositionGettersAndSetters) {
//   size_t playerID = 0;
//   Player testPlayer(playerID, "TestPlayer");

//   // Get initial position
//   Position initialPos = testPlayer.getMarblePosition(1);
//   EXPECT_TRUE(initialPos.isInHome());
//   EXPECT_EQ(initialPos.index, 1);

//   // Set new position on TRACK
//   Position newPos(BoardLocation::TRACK, 20, playerID);
//   testPlayer.setMarblePosition(1, newPos);

//   // Verify it was updated
//   Position updatedPos = testPlayer.getMarblePosition(1);
//   EXPECT_TRUE(updatedPos.equals(newPos));
//   EXPECT_FALSE(updatedPos.isInHome());

//   // Other marbles should be unchanged
//   EXPECT_TRUE(testPlayer.getMarblePosition(0).isInHome());
//   EXPECT_TRUE(testPlayer.getMarblePosition(2).isInHome());
//   EXPECT_TRUE(testPlayer.getMarblePosition(3).isInHome());
// }

// // Test Player marble position out of bounds
// TEST(PlayerTest, MarblePositionOutOfBounds) {
//   size_t playerID = 1;
//   Player testPlayer(playerID, "TestPlayer");

//   // Get with invalid index
//   EXPECT_THROW(testPlayer.getMarblePosition(4), std::out_of_range);
//   EXPECT_THROW(testPlayer.getMarblePosition(10), std::out_of_range);

//   // Set with invalid index
//   Position pos(BoardLocation::TRACK, 10, playerID);
//   EXPECT_THROW(testPlayer.setMarblePosition(4, pos), std::out_of_range);
//   EXPECT_THROW(testPlayer.setMarblePosition(100, pos), std::out_of_range);
// }

// // Test Player getMarbleIndexByPos
// TEST(PlayerTest, GetMarbleIndexByPos) {
//   size_t playerID = 2;
//   Player testPlayer(playerID, "TestPlayer");

//   // Move marble 2 to a specific position
//   Position targetPos(BoardLocation::TRACK, 35, playerID);
//   testPlayer.setMarblePosition(2, targetPos);

//   // Should find marble at that position
//   auto foundIdx = testPlayer.getMarbleIndexByPos(targetPos);
//   ASSERT_TRUE(foundIdx.has_value());
//   EXPECT_EQ(foundIdx.value(), 2);

//   // Should not find marble at non-existent position
//   Position emptyPos(BoardLocation::TRACK, 50, playerID);
//   auto notFoundIdx = testPlayer.getMarbleIndexByPos(emptyPos);
//   EXPECT_FALSE(notFoundIdx.has_value());

//   // Verify initial HOME positions can be found
//   Position homePos(BoardLocation::HOME, 0, playerID);
//   auto homeIdx = testPlayer.getMarbleIndexByPos(homePos);
//   ASSERT_TRUE(homeIdx.has_value());
//   EXPECT_EQ(homeIdx.value(), 0);
// }

// // Test Player start blocked functionality
// TEST(PlayerTest, StartBlockedFunctionality) {
//   size_t playerID = 0;
//   Player testPlayer(playerID, "TestPlayer");

//   // Initially not blocked
//   EXPECT_FALSE(testPlayer.isStartBlocked());
//   EXPECT_FALSE(testPlayer.getStartBlocked().has_value());

//   // Block with player 2's marble
//   testPlayer.setStartBlocked(2);
//   EXPECT_TRUE(testPlayer.isStartBlocked());
//   ASSERT_TRUE(testPlayer.getStartBlocked().has_value());
//   EXPECT_EQ(testPlayer.getStartBlocked().value(), 2);

//   // Reset blocked status
//   testPlayer.resetStartBlocked();
//   EXPECT_FALSE(testPlayer.isStartBlocked());
//   EXPECT_FALSE(testPlayer.getStartBlocked().has_value());
// }

// // Test Player checkFinished with all marbles in finish
// TEST(PlayerTest, CheckFinishedWithAllInFinish) {
//   size_t playerID = 1;
//   Player testPlayer(playerID, "TestPlayer");

//   // Initially not finished (all in HOME)
//   EXPECT_FALSE(testPlayer.checkFinished());

//   // Move all marbles to FINISH
//   for (size_t i = 0; i < 4; ++i) {
//     Position finishPos(BoardLocation::FINISH, i, playerID);
//     testPlayer.setMarblePosition(i, finishPos);
//   }

//   // Now should be finished
//   EXPECT_TRUE(testPlayer.checkFinished());
// }

// // Test PlayercheckFinished with mixed positions
// TEST(PlayerTest, CheckFinishedWithMixedPositions) {
//   size_t playerID = 3;
//   Player testPlayer(playerID, "TestPlayer");

//   // Move 3 marbles to FINISH, leave 1 in HOME
//   testPlayer.setMarblePosition(0, Position(BoardLocation::FINISH, 0,
//   playerID)); testPlayer.setMarblePosition(1, Position(BoardLocation::FINISH,
//   1, playerID)); testPlayer.setMarblePosition(2,
//   Position(BoardLocation::FINISH, 2, playerID));
//   // Marble 3 stays in HOME

//   EXPECT_FALSE(testPlayer.checkFinished());

//   // Move 1 marble to TRACK instead
//   testPlayer.setMarblePosition(3, Position(BoardLocation::TRACK, 48,
//   playerID)); EXPECT_FALSE(testPlayer.checkFinished());

//   // Complete the finish
//   testPlayer.setMarblePosition(3, Position(BoardLocation::FINISH, 3,
//   playerID)); EXPECT_TRUE(testPlayer.checkFinished());
// }

// // Test Player popCardFromHand error cases
// TEST(PlayerTest, PopCardFromHandErrors) {
//   size_t playerID = 0;
//   Player testPlayer(playerID, "TestPlayer");

//   // Pop from empty hand
//   EXPECT_THROW(testPlayer.popCardFromHand(0), std::out_of_range);

//   // Add some cards
//   std::vector<size_t> handCards = {5, 10, 15};
//   testPlayer.setHand(handCards);

//   // Pop with invalid index
//   EXPECT_THROW(testPlayer.popCardFromHand(3), std::out_of_range);
//   EXPECT_THROW(testPlayer.popCardFromHand(10), std::out_of_range);

//   // Valid pop should still work
//   EXPECT_NO_THROW(testPlayer.popCardFromHand(1));
// }

// // Test Player popCardFromHand normal operation
// TEST(PlayerTest, PopCardFromHandNormalOperation) {
//   size_t playerID = 0;
//   std::string playerName = "ActivePlayer";
//   Player testPlayer(playerID, playerName);

//   // Set hand with cards
//   std::vector<size_t> handCards = {4, 8, 12};
//   testPlayer.setHand(handCards);

//   // Pop a card from hand
//   size_t poppedCard = testPlayer.popCardFromHand(1);  // Should pop '8'
//   EXPECT_EQ(poppedCard, 8);
//   EXPECT_EQ(testPlayer.getHand().size(), 2);
//   EXPECT_EQ(testPlayer.getHand()[0], 4);
//   EXPECT_EQ(testPlayer.getHand()[1], 12);

//   // Pop remaining cards
//   testPlayer.popCardFromHand(0);  // Pop '4'
//   testPlayer.popCardFromHand(0);  // Pop '12'

//   // Hand should be empty now
//   EXPECT_TRUE(testPlayer.getHand().empty());
// }

// // Test Player JSON round-trip with default state
// TEST(PlayerTest, JsonSerializationDefaultState) {
//   size_t playerID = 2;
//   std::string playerName = "JsonPlayer";
//   Player originalPlayer(playerID, playerName);

//   // Serialize to JSON
//   nlohmann::json json = originalPlayer;

//   // Deserialize back
//   Player restoredPlayer = json.get<Player>();

//   // Check equality
//   EXPECT_EQ(restoredPlayer.getId(), originalPlayer.getId());
//   EXPECT_EQ(restoredPlayer.getName(), originalPlayer.getName());
//   EXPECT_EQ(restoredPlayer.getStartField(), originalPlayer.getStartField());
//   EXPECT_EQ(restoredPlayer.isActiveInRound(),
//   originalPlayer.isActiveInRound());
//   EXPECT_EQ(restoredPlayer.getHand().size(),
//   originalPlayer.getHand().size());
//   EXPECT_FALSE(restoredPlayer.isStartBlocked());

//   // Check all marble positions
//   for (size_t i = 0; i < 4; ++i) {
//     EXPECT_TRUE(restoredPlayer.getMarblePosition(i).equals(
//         originalPlayer.getMarblePosition(i)));
//   }
// }

// // Test Player JSON round-trip with modified state
// TEST(PlayerTest, JsonSerializationWithModifiedState) {
//   size_t playerID = 1;
//   std::string playerName = "ModifiedPlayer";
//   Player originalPlayer(playerID, playerName);

//   // Modify state
//   std::vector<size_t> handCards = {5, 10, 15};
//   originalPlayer.setHand(handCards);
//   originalPlayer.setActiveInRound(false);
//   originalPlayer.setMarblePosition(
//       0, Position(BoardLocation::TRACK, 20, playerID));
//   originalPlayer.setMarblePosition(
//       2, Position(BoardLocation::FINISH, 1, playerID));

//   // Serialize to JSON
//   nlohmann::json json = originalPlayer;

//   // Deserialize back
//   Player restoredPlayer = json.get<Player>();

//   // Check all state was preserved
//   EXPECT_EQ(restoredPlayer.getId(), originalPlayer.getId());
//   EXPECT_EQ(restoredPlayer.getName(), originalPlayer.getName());
//   EXPECT_EQ(restoredPlayer.isActiveInRound(), false);
//   EXPECT_EQ(restoredPlayer.getHand().size(), 3);
//   for (size_t i = 0; i < 3; ++i) {
//     EXPECT_EQ(restoredPlayer.getHand()[i], originalPlayer.getHand()[i]);
//   }

//   // Check marble positions
//   for (size_t i = 0; i < 4; ++i) {
//     EXPECT_TRUE(restoredPlayer.getMarblePosition(i).equals(
//         originalPlayer.getMarblePosition(i)));
//   }
// }

// // Test Player JSON with startBlocked set
// TEST(PlayerTest, JsonSerializationWithStartBlocked) {
//   size_t playerID = 3;
//   std::string playerName = "BlockedPlayer";
//   Player originalPlayer(playerID, playerName);

//   // Set start blocked by player 1
//   originalPlayer.setStartBlocked(1);

//   // Serialize to JSON
//   nlohmann::json json = originalPlayer;

//   // Deserialize back
//   Player restoredPlayer = json.get<Player>();

//   // Check startBlocked was preserved
//   EXPECT_TRUE(restoredPlayer.isStartBlocked());
//   ASSERT_TRUE(restoredPlayer.getStartBlocked().has_value());
//   EXPECT_EQ(restoredPlayer.getStartBlocked().value(), 1);
// }

// // Test Player JSON with startBlocked unset
// TEST(PlayerTest, JsonSerializationWithStartUnblocked) {
//   size_t playerID = 0;
//   std::string playerName = "UnblockedPlayer";
//   Player originalPlayer(playerID, playerName);

//   // Explicitly ensure not blocked (default state)
//   originalPlayer.resetStartBlocked();

//   // Serialize to JSON
//   nlohmann::json json = originalPlayer;

//   // Deserialize back
//   Player restoredPlayer = json.get<Player>();

//   // Check startBlocked is not set
//   EXPECT_FALSE(restoredPlayer.isStartBlocked());
//   EXPECT_FALSE(restoredPlayer.getStartBlocked().has_value());
// }
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "shared/game.hpp"
#include "shared/game_objects.hpp"
#include "shared/game_types.hpp"

using namespace BraendiDog;

// Test Position creation and equality
TEST(PositionTest, CreateAndCompare) {
  Position pos1(BoardLocation::TRACK, 5, 0);
  Position pos2(BoardLocation::TRACK, 5, 1);
  Position pos3(BoardLocation::HOME, 3, 0);
  Position pos4(BoardLocation::HOME, 3, 1);
  Position pos5(BoardLocation::FINISH, 2, 0);

  EXPECT_TRUE(pos1.equals(pos2));  // Same position on TRACK differs in playerID
  EXPECT_FALSE(pos1.equals(pos3));  // Different location
  EXPECT_FALSE(
      pos3.equals(pos4));  // Different playerID in "same HOME" position
}

TEST(PositionTest, InvalidPositionThrows) {
  // Invalid playerID
  EXPECT_THROW(Position(BoardLocation::HOME, 0, 4), std::out_of_range);

  // Invalid index for HOME or FINISH
  EXPECT_THROW(Position(BoardLocation::HOME, 4, 0), std::out_of_range);
  EXPECT_THROW(Position(BoardLocation::FINISH, 5, 1), std::out_of_range);

  // Invalid index for TRACK
  EXPECT_THROW(Position(BoardLocation::TRACK, 64, 2), std::out_of_range);
}

TEST(PositionTest, IsInHomeAndFinish) {
  Position posH(BoardLocation::HOME, 3, 1);
  Position posF(BoardLocation::FINISH, 2, 0);
  Position posT(BoardLocation::TRACK, 10, 2);

  // Home checks
  EXPECT_TRUE(posH.isInHome());
  EXPECT_FALSE(posH.isInFinish());
  // Finish checks
  EXPECT_FALSE(posF.isInHome());
  EXPECT_TRUE(posF.isInFinish());
  // Track checks
  EXPECT_FALSE(posT.isInHome());
  EXPECT_FALSE(posT.isInFinish());
}

// Test Position JSON round-trip
TEST(PositionTest, JsonSerialization) {
  Position original(BoardLocation::FINISH, 2, 1);

  // Serialize to JSON
  nlohmann::json json = original;

  // Deserialize back
  Position restored = json.get<Position>();

  // Should be equal
  EXPECT_TRUE(original.equals(restored));
  EXPECT_EQ(restored.index, 2);
  EXPECT_EQ(restored.playerID, 1);
}

// Test MarbleIdentifier Creation and Getters
TEST(MarbleIdentifierTest, CreateAndGetters) {
  MarbleIdentifier mid(2, 3);

  EXPECT_EQ(mid.playerID, 2);
  EXPECT_EQ(mid.marbleIdx, 3);
}

// Test MarbleIdentifier Invalid Creation
TEST(MarbleIdentifierTest, InvalidCreationThrows) {
  // Invalid playerID
  EXPECT_THROW(MarbleIdentifier(4, 0), std::out_of_range);

  // Invalid marbleIdx
  EXPECT_THROW(MarbleIdentifier(1, 4), std::out_of_range);
}

// Test MarbleIdentifier JSON round-trip
TEST(MarbleIdentifierTest, JsonSerialization) {
  MarbleIdentifier original(1, 2);

  // Serialize to JSON
  nlohmann::json json = original;

  // Deserialize back
  MarbleIdentifier restored = json.get<MarbleIdentifier>();

  // Check equality
  EXPECT_EQ(restored.playerID, original.playerID);
  EXPECT_EQ(restored.marbleIdx, original.marbleIdx);
}

// Test Move Creation and Getters
TEST(MoveTest, CreateAndGetters) {
  size_t cardID = 20;
  size_t handIndex = 3;

  Position posA(BoardLocation::TRACK, 30, 0);
  Position posB(BoardLocation::HOME, 0, 1);
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {MarbleIdentifier(0, 1), posA}, {MarbleIdentifier(1, 2), posB}};
  Move move(cardID, handIndex, movements);

  EXPECT_EQ(move.getCardID(), cardID);
  EXPECT_EQ(move.getHandIndex(), handIndex);
  EXPECT_EQ(move.getMovements().size(), movements.size());
  EXPECT_EQ(move.getMovements()[0].first.playerID, 0);
  EXPECT_EQ(move.getMovements()[0].first.marbleIdx, 1);
  EXPECT_TRUE(move.getMovements()[0].second.equals(posA));
  EXPECT_EQ(move.getMovements()[1].first.playerID, 1);
  EXPECT_EQ(move.getMovements()[1].first.marbleIdx, 2);
  EXPECT_TRUE(move.getMovements()[1].second.equals(posB));
}

// Test Move Invalid Creation
TEST(MoveTest, InvalidCreationThrows) {
  size_t handIndex = 2;
  Position pos(BoardLocation::TRACK, 10, 0);
  std::vector<std::pair<MarbleIdentifier, Position>> movements = {
      {MarbleIdentifier(0, 0), pos}};

  // Invalid cardID
  EXPECT_THROW(Move(70, handIndex, movements), std::out_of_range);
}

// Test Card creation and getters
TEST(CardTest, CreateAndGetters) {
  Rank rank = Rank::ACE;
  Suit suit = Suit::HEARTS;

  Card card(rank, suit);

  EXPECT_EQ(card.getRank(), rank);
  EXPECT_EQ(card.getSuit(), suit);
  EXPECT_EQ(card.getMoveRules().size(), 3);
  EXPECT_EQ(card.getMoveRules()[0].first, MoveType::SIMPLE);
  EXPECT_EQ(card.getMoveRules()[0].second, 1);
  EXPECT_EQ(card.getMoveRules()[1].first, MoveType::SIMPLE);
  EXPECT_EQ(card.getMoveRules()[1].second, 11);
  EXPECT_EQ(card.getMoveRules()[2].first, MoveType::START);
  EXPECT_EQ(card.getMoveRules()[2].second, 0);
}

// Test Card JSON round-trip
TEST(CardTest, JsonSerialization) {
  Rank rank = Rank::KING;
  Suit suit = Suit::SPADES;

  Card originalCard(rank, suit);

  // Serialize to JSON
  nlohmann::json json = originalCard;

  // Deserialize back
  Card restoredCard = json.get<Card>();

  // Check equality
  EXPECT_EQ(restoredCard.getRank(), originalCard.getRank());
  EXPECT_EQ(restoredCard.getSuit(), originalCard.getSuit());
  EXPECT_EQ(restoredCard.getMoveRules().size(),
            originalCard.getMoveRules().size());
  for (size_t i = 0; i < restoredCard.getMoveRules().size(); ++i) {
    EXPECT_EQ(restoredCard.getMoveRules()[i].first,
              originalCard.getMoveRules()[i].first);
    EXPECT_EQ(restoredCard.getMoveRules()[i].second,
              originalCard.getMoveRules()[i].second);
  }
}

// Test Player creation, getters and setters
TEST(PlayerTest, CreateAndGettersSetters) {
  size_t playerID = 1;
  std::string playerName = "TestPlayer";
  Player testPlayer(playerID, playerName);

  // Check initial values
  EXPECT_EQ(testPlayer.getId(), playerID);
  EXPECT_EQ(testPlayer.getName(), playerName);
  EXPECT_EQ(testPlayer.getStartField(), 16 * playerID);
  EXPECT_EQ(testPlayer.getMarbles().size(), 4);
  EXPECT_TRUE(testPlayer.isActiveInRound());
  EXPECT_TRUE(testPlayer.getHand().empty());
  EXPECT_FALSE(testPlayer.isStartBlocked());  // Initially not blocked

  // Set active in round
  testPlayer.setActiveInRound(false);
  EXPECT_FALSE(testPlayer.isActiveInRound());

  // Set hand
  std::vector<size_t> handCards = {3, 7, 11};
  testPlayer.setHand(handCards);
  EXPECT_EQ(testPlayer.getHand().size(), handCards.size());
  for (size_t i = 0; i < handCards.size(); ++i) {
    EXPECT_EQ(testPlayer.getHand()[i], handCards[i]);
  }
}

// Test Player initial marble positions
TEST(PlayerTest, InitialMarblePositions) {
  size_t playerID = 2;
  Player testPlayer(playerID, "Player2");

  // All marbles should start in HOME
  for (size_t i = 0; i < 4; ++i) {
    Position pos = testPlayer.getMarblePosition(i);
    EXPECT_TRUE(pos.isInHome());
    EXPECT_EQ(pos.boardLocation, BoardLocation::HOME);
    EXPECT_EQ(pos.index, i);
    EXPECT_EQ(pos.playerID, playerID);
  }
}

// Test Player marble position getters and setters
TEST(PlayerTest, MarblePositionGettersAndSetters) {
  size_t playerID = 0;
  Player testPlayer(playerID, "TestPlayer");

  // Get initial position
  Position initialPos = testPlayer.getMarblePosition(1);
  EXPECT_TRUE(initialPos.isInHome());
  EXPECT_EQ(initialPos.index, 1);

  // Set new position on TRACK
  Position newPos(BoardLocation::TRACK, 20, playerID);
  testPlayer.setMarblePosition(1, newPos);

  // Verify it was updated
  Position updatedPos = testPlayer.getMarblePosition(1);
  EXPECT_TRUE(updatedPos.equals(newPos));
  EXPECT_FALSE(updatedPos.isInHome());

  // Other marbles should be unchanged
  EXPECT_TRUE(testPlayer.getMarblePosition(0).isInHome());
  EXPECT_TRUE(testPlayer.getMarblePosition(2).isInHome());
  EXPECT_TRUE(testPlayer.getMarblePosition(3).isInHome());
}

// Test Player marble position out of bounds
TEST(PlayerTest, MarblePositionOutOfBounds) {
  size_t playerID = 1;
  Player testPlayer(playerID, "TestPlayer");

  // Get with invalid index
  EXPECT_THROW(testPlayer.getMarblePosition(4), std::out_of_range);
  EXPECT_THROW(testPlayer.getMarblePosition(10), std::out_of_range);

  // Set with invalid index
  Position pos(BoardLocation::TRACK, 10, playerID);
  EXPECT_THROW(testPlayer.setMarblePosition(4, pos), std::out_of_range);
  EXPECT_THROW(testPlayer.setMarblePosition(100, pos), std::out_of_range);
}

// Test Player getMarbleIndexByPos
TEST(PlayerTest, GetMarbleIndexByPos) {
  size_t playerID = 2;
  Player testPlayer(playerID, "TestPlayer");

  // Move marble 2 to a specific position
  Position targetPos(BoardLocation::TRACK, 35, playerID);
  testPlayer.setMarblePosition(2, targetPos);

  // Should find marble at that position
  auto foundIdx = testPlayer.getMarbleIndexByPos(targetPos);
  ASSERT_TRUE(foundIdx.has_value());
  EXPECT_EQ(foundIdx.value(), 2);

  // Should not find marble at non-existent position
  Position emptyPos(BoardLocation::TRACK, 50, playerID);
  auto notFoundIdx = testPlayer.getMarbleIndexByPos(emptyPos);
  EXPECT_FALSE(notFoundIdx.has_value());

  // Verify initial HOME positions can be found
  Position homePos(BoardLocation::HOME, 0, playerID);
  auto homeIdx = testPlayer.getMarbleIndexByPos(homePos);
  ASSERT_TRUE(homeIdx.has_value());
  EXPECT_EQ(homeIdx.value(), 0);
}

// Test Player start blocked functionality
TEST(PlayerTest, StartBlockedFunctionality) {
  size_t playerID = 0;
  Player testPlayer(playerID, "TestPlayer");

  // Initially not blocked
  EXPECT_FALSE(testPlayer.isStartBlocked());
  EXPECT_FALSE(testPlayer.getStartBlocked().has_value());

  // Block with player 2's marble
  testPlayer.setStartBlocked(2);
  EXPECT_TRUE(testPlayer.isStartBlocked());
  ASSERT_TRUE(testPlayer.getStartBlocked().has_value());
  EXPECT_EQ(testPlayer.getStartBlocked().value(), 2);

  // Reset blocked status
  testPlayer.resetStartBlocked();
  EXPECT_FALSE(testPlayer.isStartBlocked());
  EXPECT_FALSE(testPlayer.getStartBlocked().has_value());
}

// Test Player checkFinished with all marbles in finish
TEST(PlayerTest, CheckFinishedWithAllInFinish) {
  size_t playerID = 1;
  Player testPlayer(playerID, "TestPlayer");

  // Initially not finished (all in HOME)
  EXPECT_FALSE(testPlayer.checkFinished());

  // Move all marbles to FINISH
  for (size_t i = 0; i < 4; ++i) {
    Position finishPos(BoardLocation::FINISH, i, playerID);
    testPlayer.setMarblePosition(i, finishPos);
  }

  // Now should be finished
  EXPECT_TRUE(testPlayer.checkFinished());
}

// Test PlayercheckFinished with mixed positions
TEST(PlayerTest, CheckFinishedWithMixedPositions) {
  size_t playerID = 3;
  Player testPlayer(playerID, "TestPlayer");

  // Move 3 marbles to FINISH, leave 1 in HOME
  testPlayer.setMarblePosition(0, Position(BoardLocation::FINISH, 0, playerID));
  testPlayer.setMarblePosition(1, Position(BoardLocation::FINISH, 1, playerID));
  testPlayer.setMarblePosition(2, Position(BoardLocation::FINISH, 2, playerID));
  // Marble 3 stays in HOME

  EXPECT_FALSE(testPlayer.checkFinished());

  // Move 1 marble to TRACK instead
  testPlayer.setMarblePosition(3, Position(BoardLocation::TRACK, 48, playerID));
  EXPECT_FALSE(testPlayer.checkFinished());

  // Complete the finish
  testPlayer.setMarblePosition(3, Position(BoardLocation::FINISH, 3, playerID));
  EXPECT_TRUE(testPlayer.checkFinished());
}

// Test Player popCardFromHand error cases
TEST(PlayerTest, PopCardFromHandErrors) {
  size_t playerID = 0;
  Player testPlayer(playerID, "TestPlayer");

  // Pop from empty hand
  EXPECT_THROW(testPlayer.popCardFromHand(0), std::out_of_range);

  // Add some cards
  std::vector<size_t> handCards = {5, 10, 15};
  testPlayer.setHand(handCards);

  // Pop with invalid index
  EXPECT_THROW(testPlayer.popCardFromHand(3), std::out_of_range);
  EXPECT_THROW(testPlayer.popCardFromHand(10), std::out_of_range);

  // Valid pop should still work
  EXPECT_NO_THROW(testPlayer.popCardFromHand(1));
}

// Test Player popCardFromHand normal operation
TEST(PlayerTest, PopCardFromHandNormalOperation) {
  size_t playerID = 0;
  std::string playerName = "ActivePlayer";
  Player testPlayer(playerID, playerName);

  // Set hand with cards
  std::vector<size_t> handCards = {4, 8, 12};
  testPlayer.setHand(handCards);

  // Pop a card from hand
  size_t poppedCard = testPlayer.popCardFromHand(1);  // Should pop '8'
  EXPECT_EQ(poppedCard, 8);
  EXPECT_EQ(testPlayer.getHand().size(), 2);
  EXPECT_EQ(testPlayer.getHand()[0], 4);
  EXPECT_EQ(testPlayer.getHand()[1], 12);

  // Pop remaining cards
  testPlayer.popCardFromHand(0);  // Pop '4'
  testPlayer.popCardFromHand(0);  // Pop '12'

  // Hand should be empty now
  EXPECT_TRUE(testPlayer.getHand().empty());
}

// Test Player JSON round-trip with default state
TEST(PlayerTest, JsonSerializationDefaultState) {
  size_t playerID = 2;
  std::string playerName = "JsonPlayer";
  Player originalPlayer(playerID, playerName);

  // Serialize to JSON
  nlohmann::json json = originalPlayer;

  // Deserialize back
  Player restoredPlayer = json.get<Player>();

  // Check equality
  EXPECT_EQ(restoredPlayer.getId(), originalPlayer.getId());
  EXPECT_EQ(restoredPlayer.getName(), originalPlayer.getName());
  EXPECT_EQ(restoredPlayer.getStartField(), originalPlayer.getStartField());
  EXPECT_EQ(restoredPlayer.isActiveInRound(), originalPlayer.isActiveInRound());
  EXPECT_FALSE(restoredPlayer.isStartBlocked());

  // Check all marble positions
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_TRUE(restoredPlayer.getMarblePosition(i).equals(
        originalPlayer.getMarblePosition(i)));
  }
}

// Test Player JSON round-trip with modified state
TEST(PlayerTest, JsonSerializationWithModifiedState) {
  size_t playerID = 1;
  std::string playerName = "ModifiedPlayer";
  Player originalPlayer(playerID, playerName);

  // Modify state
  std::vector<size_t> handCards = {5, 10, 15};
  originalPlayer.setHand(handCards);
  originalPlayer.setActiveInRound(false);
  originalPlayer.setMarblePosition(
      0, Position(BoardLocation::TRACK, 20, playerID));
  originalPlayer.setMarblePosition(
      2, Position(BoardLocation::FINISH, 1, playerID));

  // Serialize to JSON
  nlohmann::json json = originalPlayer;

  // Deserialize back
  Player restoredPlayer = json.get<Player>();

  // Check all state was preserved
  EXPECT_EQ(restoredPlayer.getId(), originalPlayer.getId());
  EXPECT_EQ(restoredPlayer.getName(), originalPlayer.getName());
  EXPECT_EQ(restoredPlayer.isActiveInRound(), false);

  // Check marble positions
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_TRUE(restoredPlayer.getMarblePosition(i).equals(
        originalPlayer.getMarblePosition(i)));
  }
}

// Test Player JSON with startBlocked set
TEST(PlayerTest, JsonSerializationWithStartBlocked) {
  size_t playerID = 3;
  std::string playerName = "BlockedPlayer";
  Player originalPlayer(playerID, playerName);

  // Set start blocked by player 1
  originalPlayer.setStartBlocked(1);

  // Serialize to JSON
  nlohmann::json json = originalPlayer;

  // Deserialize back
  Player restoredPlayer = json.get<Player>();

  // Check startBlocked was preserved
  EXPECT_TRUE(restoredPlayer.isStartBlocked());
  ASSERT_TRUE(restoredPlayer.getStartBlocked().has_value());
  EXPECT_EQ(restoredPlayer.getStartBlocked().value(), 1);
}

// Test Player JSON with startBlocked unset
TEST(PlayerTest, JsonSerializationWithStartUnblocked) {
  size_t playerID = 0;
  std::string playerName = "UnblockedPlayer";
  Player originalPlayer(playerID, playerName);

  // Explicitly ensure not blocked (default state)
  originalPlayer.resetStartBlocked();

  // Serialize to JSON
  nlohmann::json json = originalPlayer;

  // Deserialize back
  Player restoredPlayer = json.get<Player>();

  // Check startBlocked is not set
  EXPECT_FALSE(restoredPlayer.isStartBlocked());
  EXPECT_FALSE(restoredPlayer.getStartBlocked().has_value());
}
