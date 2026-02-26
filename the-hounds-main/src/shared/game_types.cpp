#include "shared/game_types.hpp"

#include <cstddef>
#include <iostream>
#include <nlohmann/json.hpp>

#include "shared/game_objects.hpp"

namespace BraendiDog {

// Position Methods

// Position Constructor
Position::Position(BoardLocation loc, size_t idx, size_t pID)
    : boardLocation(loc), index(idx), playerID(pID) {
  if (pID > 3 || (loc != BoardLocation::TRACK && idx > 3) ||
      (loc == BoardLocation::TRACK && idx > 63)) {
    throw std::out_of_range("Invalid Board Position");
  }
}

// Position Equality Check
bool Position::equals(const Position& other) const {
  return (boardLocation == other.boardLocation) && (index == other.index) &&
         ((playerID == other.playerID) ||
          (boardLocation == BoardLocation::TRACK));
}

// isInHome Method
bool Position::isInHome() const { return boardLocation == BoardLocation::HOME; }

// isInFinish Method
bool Position::isInFinish() const {
  return boardLocation == BoardLocation::FINISH;
}

// MarbleIdentifier Methods

// MarbleIdentifier
MarbleIdentifier::MarbleIdentifier(size_t pID, size_t mIdx)
    : playerID(pID), marbleIdx(mIdx) {
  if (pID > 3 || mIdx > 3) {
    throw std::out_of_range("Invalid marble identifier");
  }
}

// Move Methods

// Move Constructor
Move::Move(size_t cID, size_t hIndex,
           std::vector<std::pair<MarbleIdentifier, Position>> moves)
    : cardID(cID), handIndex(hIndex), movements(moves) {
  if (cID > 54) {
    throw std::out_of_range("Invalid card index");
  }
}

// Get Card ID
size_t Move::getCardID() const { return cardID; }

// Get Hand Index
size_t Move::getHandIndex() const { return handIndex; }

// Get Movements
const std::vector<std::pair<MarbleIdentifier, Position>>& Move::getMovements()
    const {
  return movements;
}
}  // namespace BraendiDog