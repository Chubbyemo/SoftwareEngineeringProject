#include "shared/game_objects.hpp"

#include <cstddef>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

namespace BraendiDog {

// Card Methods

// Card Constructor
Card::Card(Rank r, Suit s) : rank(r), suit(s) {
  switch (r) {
    case Rank::ACE:
      moveRules.emplace_back(MoveType::SIMPLE, 1);
      moveRules.emplace_back(MoveType::SIMPLE, 11);
      moveRules.emplace_back(MoveType::START, 0);
      break;
    case Rank::TWO:
      moveRules.emplace_back(MoveType::SIMPLE, 2);
      break;
    case Rank::THREE:
      moveRules.emplace_back(MoveType::SIMPLE, 3);
      break;
    case Rank::FOUR:
      moveRules.emplace_back(MoveType::SIMPLE, 4);
      moveRules.emplace_back(MoveType::SIMPLE, -4);
      break;
    case Rank::FIVE:
      moveRules.emplace_back(MoveType::SIMPLE, 5);
      break;
    case Rank::SIX:
      moveRules.emplace_back(MoveType::SIMPLE, 6);
      break;
    case Rank::SEVEN:
      moveRules.emplace_back(MoveType::SEVEN, 7);
      break;
    case Rank::EIGHT:
      moveRules.emplace_back(MoveType::SIMPLE, 8);
      break;
    case Rank::NINE:
      moveRules.emplace_back(MoveType::SIMPLE, 9);
      break;
    case Rank::TEN:
      moveRules.emplace_back(MoveType::SIMPLE, 10);
      break;
    case Rank::JACK:
      moveRules.emplace_back(MoveType::SWAP, 0);
      break;
    case Rank::QUEEN:
      moveRules.emplace_back(MoveType::SIMPLE, 12);
      break;
    case Rank::KING:
      moveRules.emplace_back(MoveType::SIMPLE, 13);
      moveRules.emplace_back(MoveType::START, 0);
      break;
    case Rank::JOKER:
      moveRules.emplace_back(MoveType::JOKER, 0);
      break;
    default:
      throw std::invalid_argument("Invalid card rank");
  }
}

// Get Card Rank
Rank Card::getRank() const { return rank; }

// Get Card Suit
Suit Card::getSuit() const { return suit; }

// Get Card Move Rules
const std::vector<std::pair<MoveType, int>>& Card::getMoveRules() const {
  return moveRules;
}

// Player Methods

// Player Constructor
Player::Player(size_t playerID, const std::string& playerName)
    : id(playerID),
      name(playerName),
      startField(16 * playerID),
      startBlocked(std::nullopt),
      activeInRound(true),
      activeInGame(true),
      marbles{
          Position(BoardLocation::HOME, 0, playerID),
          Position(BoardLocation::HOME, 1, playerID),
          Position(BoardLocation::HOME, 2, playerID),
          Position(BoardLocation::HOME, 3, playerID),
      },
      hand(std::vector<size_t>()) {}

// Get Player ID
size_t Player::getId() const { return id; }

// Get Player Name
std::string Player::getName() const { return name; }

// Get Player Start Field
size_t Player::getStartField() const { return startField; }

std::optional<size_t> Player::getStartBlocked() const { return startBlocked; }

// Get Player Start Blocked Status
bool Player::isStartBlocked() const { return startBlocked.has_value(); }

// Check if Player is Active in Round
bool Player::isActiveInRound() const { return activeInRound; }

// Check if Player is Active in Game
bool Player::isActiveInGame() const { return activeInGame; }

// Get Player Marbles
const std::array<Position, 4>& Player::getMarbles() const { return marbles; }

// Get Marble Position by Index
const Position& Player::getMarblePosition(size_t marbleIndex) const {
  if (marbleIndex >= marbles.size()) {
    throw std::out_of_range("Marble index out of range");
  }
  return marbles[marbleIndex];
}

// Get Marble Index by Position
std::optional<size_t> Player::getMarbleIndexByPos(const Position& pos) const {
  for (size_t i = 0; i < marbles.size(); ++i) {
    if (marbles[i].equals(pos)) {
      return i;
    }
  }
  return std::nullopt;
}

// Get Player Hand
const std::vector<size_t>& Player::getHand() const { return hand; }

// Check if Player's Hand is Empty
bool Player::isHandEmpty() const { return hand.empty(); }

// Check if Player has Joker in Hand
bool Player::hasJokerInHand() const {
  for (const auto& cardId : hand) {
    if (cardId == 52 || cardId == 53) {
      return true;
    }
  }
  return false;
}

// Check if Player has Seven in Hand
bool Player::hasCardInHand(size_t rank) const {
  for (const auto& cardId : hand) {
    if (cardId % 13 + 1 == rank && cardId < 52) {
      return true;
    }
  }
  return false;
}

// Set Marble Position by Index
void Player::setMarblePosition(size_t marbleIndex,
                               const Position& newPosition) {
  if (marbleIndex >= marbles.size()) {
    throw std::out_of_range("Marble index out of range");
  }
  marbles[marbleIndex] = newPosition;
}

// Set Player Active in Round
void Player::setActiveInRound(bool isActive) { activeInRound = isActive; }

// Set Player Active in Game
void Player::setActiveInGame(bool isActive) { activeInGame = isActive; }

// Set Player Start Blocked Status
void Player::setStartBlocked(size_t blocked) { startBlocked = blocked; }

// Reset Player Start Blocked Status
void Player::resetStartBlocked() { startBlocked = std::nullopt; }

// Set Player Hand
void Player::setHand(const std::vector<size_t>& cardIds) { hand = cardIds; }

// Pop Card From Hand
size_t Player::popCardFromHand(size_t handIndex) {
  if (handIndex >= hand.size()) {
    throw std::out_of_range("Hand index out of range");
  }
  size_t cardId = hand[handIndex];
  hand.erase(hand.begin() + handIndex);
  return cardId;
}

// Check if Player has Finished
bool Player::checkFinished() const {
  for (const auto& marble : marbles) {
    if (!marble.isInFinish()) {
      return false;
    }
  }
  return true;
}
}  // namespace BraendiDog