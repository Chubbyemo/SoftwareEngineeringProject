#include "client/MovePhaseController.hpp"

#include <iostream>
#include <stdexcept>

MovePhaseController::MovePhaseController(Client* client, size_t myPlayerIndex,
                                         BraendiDog::GameState& gameState)
    : client_(client),
      myPlayerIndex_(myPlayerIndex),
      gameState_(gameState),
      selectedHandIndex_(-1),
      selectedCardID_(-1),
      selectedMarble_(std::nullopt),
      legalMoves_({}),
      filteredMoves_({}),
      jokerMoves_({}),
      sevenMoves_({}),
      sevenTempGameState_(std::nullopt),
      builtSevenMove_(),
      totalSevenMoveValue_(0),
      jokerSelectedRank_(-1) {}

void MovePhaseController::onCardClicked(int handIndex) {
  if (!gameState_.isMyTurn(myPlayerIndex_)) {
    if (statusCallback) statusCallback("It's not your turn.");
    return;
  }

  // If clicking the same card, deselect everything (unless it's a Joker)
  size_t clickedCardID =
      gameState_.getPlayers()[myPlayerIndex_].value().getHand()[handIndex];
  bool isJoker = (clickedCardID == 52 || clickedCardID == 53);

  if (selectedHandIndex_ == handIndex && !isJoker) {
    clearSelection();
    if (statusCallback) statusCallback("Selection cleared.");
    return;
  }

  // SELECT/REPLACE card (even if marble was selected)
  selectedHandIndex_ = handIndex;
  selectedCardID_ =
      gameState_.getPlayers()[myPlayerIndex_].value().getHand()[handIndex];
  const BraendiDog::Card& selectedCard = gameState_.getDeck()[selectedCardID_];

  // Initialize Seven temp state if Seven card OR Joker-as-Seven selected
  bool isSevenCard = (selectedCard.getRank() == BraendiDog::Rank::SEVEN) ||
                     (selectedCard.getRank() == BraendiDog::Rank::JOKER &&
                      jokerSelectedRank_ == 7);

  if (isSevenCard) {
    sevenTempGameState_ = gameState_;  // Create copy of current state
    totalSevenMoveValue_ = 0;
    builtSevenMove_ = BraendiDog::Move();  // Reset built move

    if (statusCallback) {
      statusCallback(
          "Seven card selected. Use all 7 steps by selecting marbles.");
    }
  } else {
    sevenTempGameState_.reset();  // Clear temp state if not Seven
  }

  selectedMarble_.reset();  // Clear marble selection when switching cards

  // Filter legal moves by selected card
  filterByCard(handIndex);

  if (statusCallback) {
    statusCallback("Card selected. Now select a marble.");
  }
  if (selectionChangedCallback) selectionChangedCallback();
}

std::vector<BraendiDog::Position> MovePhaseController::getPossibleDestinations()
    const {
  std::vector<BraendiDog::Position> destinations;

  // No marble selected, no destinations
  if (!selectedMarble_.has_value()) {
    return destinations;
  }

  // get all possible destinations from filteredMoves_
  for (const auto& move : filteredMoves_) {
    if (move.getMovements().empty()) continue;

    const BraendiDog::Position& dest = move.getMovements()[0].second;

    bool alreadyAdded = false;
    for (const auto& existing : destinations) {
      if (existing.boardLocation == dest.boardLocation &&
          existing.index == dest.index &&
          (dest.boardLocation == BraendiDog::BoardLocation::TRACK ||
           existing.playerID == dest.playerID)) {
        alreadyAdded = true;
        break;
      }
    }

    if (!alreadyAdded) {
      destinations.push_back(dest);
    }
  }

  return destinations;
}

void MovePhaseController::onBoardPositionClicked(
    const BraendiDog::Position& pos) {
  if (!gameState_.isMyTurn(myPlayerIndex_)) {
    if (statusCallback) statusCallback("It's not your turn.");
    return;
  }

  // Use temp state if Seven move is in progress, otherwise use regular state
  const auto& stateToCheck = sevenTempGameState_.has_value()
                                 ? sevenTempGameState_.value()
                                 : gameState_;
  auto marbleId = stateToCheck.isFieldOccupied(pos);

  // CASE 1: No card selected - ignore clicks
  if (selectedHandIndex_ == -1) {
    if (statusCallback) statusCallback("Select a card first.");
    return;
  }

  // CASE 2: Card selected - either selecting or switching marble
  if (marbleId.has_value() && marbleId->playerID == myPlayerIndex_) {
    // Reset to card-filtered moves
    filterByCard(selectedHandIndex_);

    // Try to filter by this marble
    bool hasMovesForMarble = filterByMarble(marbleId.value());

    if (!hasMovesForMarble) {
      if (statusCallback) {
        statusCallback("This marble can't be moved with the selected card.");
      }
      return;  // Don't select the marble
    }

    // Commit the selection
    selectedMarble_ = marbleId.value();

    if (statusCallback) {
      statusCallback(
          "Marble selected. Click destination or different marble to switch.");
    }
    if (selectionChangedCallback) selectionChangedCallback();
    return;
  }

  // CASE 3: Card selected, trying to click destination without marble
  if (!selectedMarble_.has_value()) {
    if (statusCallback) statusCallback("Select a marble first.");
    return;
  }

  // CASE 4: Card AND marble selected, clicked a destination - SUBMIT
  auto matchingMove = findMatchingMove(pos);
  if (!matchingMove.has_value()) {
    if (statusCallback) statusCallback("No legal move to that destination.");
    return;
  }

  // Check if this is a Seven card
  size_t selectedCardID = gameState_.getPlayers()[myPlayerIndex_]
                              .value()
                              .getHand()[selectedHandIndex_];
  const BraendiDog::Card& selectedCard = gameState_.getDeck()[selectedCardID];

  bool isSevenMove = (selectedCard.getRank() == BraendiDog::Rank::SEVEN) ||
                     (selectedCard.getRank() == BraendiDog::Rank::JOKER &&
                      jokerSelectedRank_ == 7);

  if (isSevenMove) {
    // Get current position from temp state (or original if first move)
    const auto& currentPos =
        sevenTempGameState_.has_value()
            ? sevenTempGameState_->getPlayers()[selectedMarble_->playerID]
                  .value()
                  .getMarblePosition(selectedMarble_->marbleIdx)
            : gameState_.getPlayers()[selectedMarble_->playerID]
                  .value()
                  .getMarblePosition(selectedMarble_->marbleIdx);

    // Calculate steps for this move
    size_t moveValue = calculateMoveSteps(currentPos, pos);
    totalSevenMoveValue_ += moveValue;

    std::cout << "DEBUG: clicked pos = (" << static_cast<int>(pos.boardLocation)
              << ", " << pos.index << ")" << std::endl;
    std::cout << "DEBUG: calculated moveValue = " << moveValue << std::endl;
    std::cout << "DEBUG: move has "
              << matchingMove.value().getMovements().size() << " movements"
              << std::endl;
    for (const auto& mv : matchingMove.value().getMovements()) {
      std::cout << "  Movement: player " << mv.first.playerID << " marble "
                << mv.first.marbleIdx << " to ("
                << static_cast<int>(mv.second.boardLocation) << ", "
                << mv.second.index << ")" << std::endl;
    }

    // Append to built Seven move
    if (builtSevenMove_.getMovements().empty()) {
      builtSevenMove_ = matchingMove.value();
    } else {
      auto existingMovements = builtSevenMove_.getMovements();
      auto newMovements = matchingMove.value().getMovements();
      existingMovements.insert(existingMovements.end(), newMovements.begin(),
                               newMovements.end());
      builtSevenMove_ =
          BraendiDog::Move(builtSevenMove_.getCardID(),
                           builtSevenMove_.getHandIndex(), existingMovements);
    }

    // Apply move to temp state for visual feedback
    if (sevenTempGameState_.has_value()) {
      sevenTempGameState_->applyTempSevenMove(matchingMove.value());
    }

    // Check if Seven is complete
    if (totalSevenMoveValue_ == 7) {
      // Submit the complete Seven move
      client_->sendPlayCard(builtSevenMove_);
      if (statusCallback) {
        statusCallback("Seven move completed! Waiting for server...");
      }

      // Clear Seven state
      sevenTempGameState_.reset();
      totalSevenMoveValue_ = 0;
      builtSevenMove_ = BraendiDog::Move();
      clearSelection();
      return;
    } else if (totalSevenMoveValue_ > 7) {
      // Invalid
      std::cerr << "Error: Seven move exceeded 7 steps." << std::endl;
      if (statusCallback) {
        statusCallback("Invalid move: exceeded 7 steps. Selection cleared.");
      }
      clearSelection();
    } else {
      // More steps needed
      size_t remainingValue = 7 - totalSevenMoveValue_;

      // Based on remaining value find synthetic cardID for Seven computation
      size_t syntheticCardID = remainingValue - 1;  // 0-6 for 1-7 steps

      // Clear marble selection for next part
      selectedMarble_.reset();

      // Recompute remaining moves with updated temp state
      if (sevenTempGameState_.has_value()) {
        sevenMoves_ = sevenTempGameState_->computeLegalMoves(
            std::array<size_t, 3>{syntheticCardID,
                                  static_cast<size_t>(selectedHandIndex_),
                                  static_cast<size_t>(selectedCardID_)},
            true);
        filterByCard(selectedHandIndex_);
      }

      if (statusCallback) {
        statusCallback("Seven: " + std::to_string(totalSevenMoveValue_) +
                       "/7 steps used. Select next marble.");
      }
      if (selectionChangedCallback) selectionChangedCallback();
      return;
    }
  }

  // Normal (non-Seven) move submission
  client_->sendPlayCard(matchingMove.value());
  if (statusCallback) statusCallback("Move submitted, waiting for server...");

  clearSelection();
}

int MovePhaseController::getSelectedHandIndex() const {
  return selectedHandIndex_;
}

std::optional<BraendiDog::MarbleIdentifier>
MovePhaseController::getSelectedMarble() const {
  return selectedMarble_;
}

void MovePhaseController::setLegalMoves(
    const std::vector<BraendiDog::Move> moves) {
  legalMoves_ = moves;
  filteredMoves_.clear();
  jokerMoves_.clear();
  sevenMoves_.clear();
  builtSevenMove_ = BraendiDog::Move();
  totalSevenMoveValue_ = 0;
  sevenTempGameState_.reset();
}

void MovePhaseController::filterByCard(int handIndex) {
  filteredMoves_.clear();

  size_t selectedCardID =
      gameState_.getPlayers()[myPlayerIndex_].value().getHand()[handIndex];
  const BraendiDog::Card& selectedCard = gameState_.getDeck()[selectedCardID];

  // Check if this is a Seven card OR a Joker mimicking a Seven
  bool isSevenMove = (selectedCard.getRank() == BraendiDog::Rank::SEVEN) ||
                     (selectedCard.getRank() == BraendiDog::Rank::JOKER &&
                      jokerSelectedRank_ == 7);

  if (isSevenMove) {
    filteredMoves_ = sevenMoves_;
    std::cout << "  -> Using sevenMoves_" << std::endl;
  } else if (selectedCard.getRank() == BraendiDog::Rank::JOKER) {  // Joker
    filteredMoves_ = jokerMoves_;
    std::cout << "  -> Using jokerMoves_" << std::endl;
  } else {
    std::cout << "  -> Filtering legalMoves_ by handIndex" << std::endl;
    // filters legal Moves by handIndex
    for (const auto& move : legalMoves_) {
      if (move.getHandIndex() == static_cast<size_t>(handIndex)) {
        filteredMoves_.push_back(move);
      } else if (!filteredMoves_.empty()) {
        break;
      }
    }
  }

  std::cout << "  Result: filteredMoves_ size: " << filteredMoves_.size()
            << std::endl;

  if (filteredMoves_.empty()) {
    if (statusCallback) statusCallback("No legal moves for selected card.");
  } else {
    if (statusCallback) statusCallback("Filtered moves by selected card.");
    std::cout << "After card filter: " << filteredMoves_.size() << " moves"
              << std::endl;
  }
}

bool MovePhaseController::filterByMarble(
    const BraendiDog::MarbleIdentifier& marbleId) {
  std::vector<BraendiDog::Move> newFiltered;

  for (const auto& move : filteredMoves_) {
    if (move.getMovements()[0].first.playerID == marbleId.playerID &&
        move.getMovements()[0].first.marbleIdx == marbleId.marbleIdx) {
      newFiltered.push_back(move);
    }
  }

  if (newFiltered.empty()) {
    // Don't update filteredMoves_, return failure
    return false;
  }

  filteredMoves_ = newFiltered;

  if (statusCallback) statusCallback("Filtered moves by selected marble.");
  std::cout << "After marble filter: " << filteredMoves_.size() << " moves"
            << std::endl;

  return true;
}

std::optional<BraendiDog::Move> MovePhaseController::findMatchingMove(
    const BraendiDog::Position& dest) const {
  for (const auto& move : filteredMoves_) {
    const auto& moveDest = move.getMovements()[0].second;

    // Check boardLocation and index
    bool locationMatches = (moveDest.boardLocation == dest.boardLocation &&
                            moveDest.index == dest.index);

    // For HOME and FINISH, also check playerID
    if (dest.boardLocation == BraendiDog::BoardLocation::HOME ||
        dest.boardLocation == BraendiDog::BoardLocation::FINISH) {
      locationMatches = locationMatches && (moveDest.playerID == dest.playerID);
    }

    if (locationMatches) {
      return move;
    }
  }
  return std::nullopt;
}

void MovePhaseController::clearSelection() {
  selectedHandIndex_ = -1;
  selectedCardID_ = -1;
  jokerSelectedRank_ = -1;
  jokerMoves_.clear();
  selectedMarble_.reset();
  filteredMoves_.clear();

  // Clear Seven state
  sevenTempGameState_.reset();
  totalSevenMoveValue_ = 0;
  builtSevenMove_ = BraendiDog::Move();

  if (selectionChangedCallback) selectionChangedCallback();
}

void MovePhaseController::foldTurn() {
  client_->sendSkipTurn();
  if (statusCallback)
    statusCallback("No moves available, waiting for server...");
}

void MovePhaseController::setJokerRank(int rank, size_t jokerHandIndex) {
  jokerSelectedRank_ = rank;

  // Update joker moves
  jokerMoves_.clear();

  // If no joker rank selected return
  if (jokerSelectedRank_ == -1) {
    return;
  }

  size_t selectedCardID =
      jokerSelectedRank_ -
      1;  // Joker rank to cardID (0-12) (always same suit but arbitrary)
  size_t jID = gameState_.getPlayers()[myPlayerIndex_]
                   .value()
                   .getHand()[jokerHandIndex];  // get joker cardID from hand
  jokerMoves_ = gameState_.computeLegalMoves(std::array<size_t, 3>{
      selectedCardID, jokerHandIndex,
      jID});  // Pass joker cardID and hand index and jID (52 or 53)
}

int MovePhaseController::getJokerRank() const { return jokerSelectedRank_; }

void MovePhaseController::clearJokerRank() { jokerSelectedRank_ = -1; }

void MovePhaseController::setSevenMoves(std::vector<BraendiDog::Move> moves) {
  sevenMoves_ = std::move(moves);
}

std::optional<BraendiDog::GameState> MovePhaseController::getSevenTempState()
    const {
  return sevenTempGameState_;
}

size_t MovePhaseController::getTotalSevenMoveValue() const {
  return totalSevenMoveValue_;
}

size_t MovePhaseController::calculateMoveSteps(
    const BraendiDog::Position& from, const BraendiDog::Position& to) const {
  // For FINISH area (marble moving within finish)
  if (from.boardLocation == BraendiDog::BoardLocation::FINISH &&
      to.boardLocation == BraendiDog::BoardLocation::FINISH) {
    return to.index - from.index;
  }

  // For TRACK to FINISH
  if (from.boardLocation == BraendiDog::BoardLocation::TRACK &&
      to.boardLocation == BraendiDog::BoardLocation::FINISH) {
    size_t startIdx =
        gameState_.getPlayers()[myPlayerIndex_].value().getStartField();

    // Calculate distance to start field
    size_t distToStart;
    if (startIdx >= from.index) {
      distToStart = startIdx - from.index;
    } else {
      distToStart = 64 - from.index + startIdx;  // Wrap around
    }

    // Distance = to start + into finish + 1 (for start field itself)
    return distToStart + to.index + 1;
  }

  // For TRACK to TRACK (normal walking)
  if (from.boardLocation == BraendiDog::BoardLocation::TRACK &&
      to.boardLocation == BraendiDog::BoardLocation::TRACK) {
    if (to.index >= from.index) {
      return to.index - from.index;
    } else {
      return 64 - from.index + to.index;  // Wrap around
    }
  }

  return 0;
}