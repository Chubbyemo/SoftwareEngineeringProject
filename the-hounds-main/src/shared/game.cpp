#include "shared/game.hpp"

#include <algorithm>  // for std::shuffle, std::sort
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>  // for std::iota
#include <random>   // for std::random_device, std::mt19937

namespace BraendiDog {

// Constructor
GameState::GameState(
    const std::array<std::optional<std::string>, 4>& gamePlayers) {
  currentPlayer = 0;     // First player to start always player 0
  roundStartPlayer = 0;  // First player to start always player 0
  roundCardCount = 6;    // Initial card count per player -> Round 1 = 6 cards
  lastPlayedCard = std::nullopt;
  leaderBoard.fill(std::nullopt);  // Initialize with invalid player IDs

  // Initialise players
  players.fill(std::nullopt);
  for (size_t i = 0; i < gamePlayers.size(); ++i) {
    if (gamePlayers[i].has_value()) {
      players[i] = Player(i, gamePlayers[i].value());
    }
  }

  // Initialise deck
  deck = std::array<Card, 54>();
  size_t cardIdx = 0;
  // Ace, 2, ..., King (Clubs, Diamonds, Hearts, Spades) + 2 Jokers
  for (size_t s = 0; s < 4; ++s) {
    for (size_t r = 0; r < 13; ++r) {
      deck[cardIdx++] = Card(static_cast<Rank>(r), static_cast<Suit>(s));
    }
  }

  // Add 2 Jokers
  deck[cardIdx++] = Card(Rank::JOKER, Suit::JOKER);
  deck[cardIdx++] = Card(Rank::JOKER, Suit::JOKER);
};

//// Getters ////

// Get deck
const std::array<Card, 54>& GameState::getDeck() const { return deck; }

// Get players
const std::array<std::optional<BraendiDog::Player>, 4>& GameState::getPlayers()
    const {
  return players;
}

std::array<std::optional<BraendiDog::Player>, 4>& GameState::getPlayers() {
  return players;
}

// Get player by index
const std::optional<BraendiDog::Player>& GameState::getPlayerByIndex(
    size_t index) const {
  return players[index];
}
std::optional<BraendiDog::Player>& GameState::getPlayerByIndex(size_t index) {
  return players[index];
}

// Get current player index
size_t GameState::getCurrentPlayer() const { return currentPlayer; }

// Get round start player index
size_t GameState::getRoundStartPlayer() const { return roundStartPlayer; }

// Get round card count
size_t GameState::getRoundCardCount() const { return roundCardCount; }

// Get last played card
std::optional<size_t> GameState::getLastPlayedCard() const {
  return lastPlayedCard;
}

// Get leaderboard
const std::array<std::optional<int>, 4>& GameState::getLeaderBoard() const {
  return leaderBoard;
}

//// Setters ////

// Set current player index
void GameState::setCurrentPlayer(size_t playerIndex) {
  currentPlayer = playerIndex;
}

// Set current player index to next active in round
void GameState::updateCurrentPlayer() {
  // Go to next player in players array with activeInRound true
  size_t nextPlayer = (currentPlayer + 1) % players.size();
  while (true) {
    if (players[nextPlayer].has_value() &&
        players[nextPlayer]->isActiveInRound()) {
      currentPlayer = nextPlayer;
      break;
    }
    nextPlayer = (nextPlayer + 1) % players.size();
  }
}

// Set round start player index
void GameState::updateRoundStartPlayer() {
  // Go to next player in players array with activeInGame true
  size_t nextPlayer = (roundStartPlayer + 1) % players.size();
  while (true) {
    if (players[nextPlayer].has_value() &&
        players[nextPlayer]->isActiveInGame()) {
      roundStartPlayer = nextPlayer;
      break;
    }
    nextPlayer = (nextPlayer + 1) % players.size();
  }
}

// Set round card count
void GameState::updateRoundCardCount() {
  if (roundCardCount > 2) {
    roundCardCount--;
  } else {
    roundCardCount = 6;  // Reset to 6 after reaching minimum of 2
  }
}

// Set last played card
void GameState::setLastPlayedCard(std::optional<size_t> cardID) {
  lastPlayedCard = cardID;
}

// Add player to leaderboard as finished
void GameState::addLeaderBoardFinished(size_t playerID) {
  // get max rank in leaderboard
  // if all are nullopt or -1, next rank is 1
  // if int >= 0, next rank is max + 1
  int nextRank = std::max_element(leaderBoard.begin(), leaderBoard.end(),
                                  [](const std::optional<int>& a,
                                     const std::optional<int>& b) {
                                    int valA = a.has_value() ? a.value() : 0;
                                    int valB = b.has_value() ? b.value() : 0;
                                    return valA < valB;
                                  })
                     ->value_or(0) +
                 1;
  leaderBoard[playerID] = nextRank;
}

// Add player to leaderboard as unfinished
void GameState::addLeaderBoardUnfinished(size_t playerID) {
  leaderBoard[playerID] = 0;  // Indicate unfinished with 0
}

// Add player to leaderboard as disconnected
void GameState::addLeaderBoardDisconnected(size_t playerID) {
  if (!leaderBoard[playerID].has_value()) {
    leaderBoard[playerID] = -1;  // Indicate disconnection with -1
  }
}

// Update a players attributes if disconnected
void GameState::disconnectPlayer(size_t playerIndex) {
  auto& playerOpt = players[playerIndex];
  if (playerOpt.has_value()) {
    Player& player = playerOpt.value();
    if (player.getId() == playerIndex) {
      // Update current player to next active in round
      updateCurrentPlayer();
    }
    player.setActiveInGame(false);
    player.setActiveInRound(false);
    // Clear player's hand and reset marbles
    player.setHand({});
    // Set all track marbles to home
    for (size_t mIdx = 0; mIdx < 4; ++mIdx) {
      if (player.getMarblePosition(mIdx).boardLocation !=
          BoardLocation::TRACK) {
        continue;  // Only reset marbles on track
      }
      Position homePos(BoardLocation::HOME, mIdx, playerIndex);
      player.setMarblePosition(mIdx, homePos);
    }
    // Add to leaderboard as disconnected
    addLeaderBoardDisconnected(playerIndex);
  }

  bool gameEnded = checkGameEnd();

  // Check if game has ended
  if (gameEnded) {
    // Add remaining active player to leaderboard
    size_t remainingPlayer =
        getActivePlayerIndices()[0];  // only one remaining player
    addLeaderBoardUnfinished(remainingPlayer);
  }
}

/// Methods ///

// Is it the turn of the specified player
bool GameState::isMyTurn(size_t playerIndex) const {
  return currentPlayer == playerIndex;
}

// Get count of active players in round
size_t GameState::getActiveInRoundCount() const {
  size_t count = 0;
  for (const auto& playerOpt : players) {
    if (playerOpt.has_value() && playerOpt->isActiveInRound()) {
      count++;
    }
  }
  return count;
}

// Get count of active players in game
size_t GameState::getActiveInGameCount() const {
  size_t count = 0;
  for (const auto& playerOpt : players) {
    if (playerOpt.has_value() && playerOpt->isActiveInGame()) {
      count++;
    }
  }
  return count;
}

// Check if the current round has ended
bool GameState::checkRoundEnd() const {
  return getActiveInRoundCount() == 0;  // All players have played their cards
}

// Check if the game has ended
bool GameState::checkGameEnd() const {
  return getActiveInGameCount() <= 1;  // Only one or no active players left
}

// Get indices of active players in game
std::vector<size_t> GameState::getActivePlayerIndices() const {
  std::vector<size_t> activeIndices = std::vector<size_t>();
  for (size_t i = 0; i < players.size(); ++i) {
    if (players[i].has_value() && players[i]->isActiveInGame()) {
      activeIndices.push_back(i);
    }
  }
  return activeIndices;
}

// Deal cards to players
std::map<size_t, std::vector<size_t>> GameState::dealCards() const {
  std::map<size_t, std::vector<size_t>> dealtCards;
  // Collect active players
  std::vector<size_t> activePlayers = getActivePlayerIndices();

  // Create random number generator
  static std::random_device rd;
  static std::mt19937 gen(rd());

  // Create a shuffled deck of card indices
  std::vector<size_t> cardIndices(deck.size());
  std::iota(cardIndices.begin(), cardIndices.end(), 0);
  std::shuffle(cardIndices.begin(), cardIndices.end(), gen);

  // Deal first N cards
  size_t cardIdx = 0;
  for (size_t playerID : activePlayers) {
    dealtCards[playerID] =
        std::vector<size_t>(cardIndices.begin() + cardIdx,
                            cardIndices.begin() + cardIdx + roundCardCount);
    cardIdx += roundCardCount;
  }

  // After dealing cards to each player, sort their hand
  for (size_t playerID : activePlayers) {
    std::sort(dealtCards[playerID].begin(), dealtCards[playerID].end(),
              [](size_t a, size_t b) {
                // Jokers (52, 53) sort to the end
                if (a >= 52 && b >= 52)
                  return a < b;             // Both jokers, keep order
                if (a >= 52) return false;  // a is joker, b comes first
                if (b >= 52) return true;   // b is joker, a comes first

                // Regular cards: sort by rank (idx % 13)
                int rankA = a % 13, rankB = b % 13;
                if (rankA != rankB) return rankA < rankB;
                return a < b;  // Same rank, sort by suit (a/13)
              });
  }

  return dealtCards;
}

//// Move Validation and Computation ////

// Field occupied check helper
std::optional<BraendiDog::MarbleIdentifier> GameState::isFieldOccupied(
    const Position& pos) const {
  // For each player
  for (size_t pID = 0; pID < 4; ++pID) {
    const auto& playerOpt = players[pID];
    if (!playerOpt.has_value()) {
      continue;  // Skip absent players
    }
    // For each marble of player
    for (size_t mIdx = 0; mIdx < 4; ++mIdx) {
      const Position& marblePos = playerOpt->getMarblePosition(mIdx);

      // For TRACK, ignore playerID in comparison
      bool matches = false;
      if (pos.boardLocation == BoardLocation::TRACK) {
        matches = (marblePos.boardLocation == pos.boardLocation &&
                   marblePos.index == pos.index);
      } else {
        // For HOME/FINISH, use full comparison including playerID
        matches = pos.equals(marblePos);
      }

      if (matches) {
        return MarbleIdentifier(pID, mIdx);
      }
    }
  }

  // No return reached -> not occupied
  return std::nullopt;
}

// Check START
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::checkStartMove(const Position& marblePos) const {
  return checkSimpleMove(
      marblePos, 0);  // Start move indicated by int value 0 in SIMPLE move
}

// Check SIMPLE
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::checkSimpleMove(const Position& marblePos, int moveValue) const {
  // ============================================================================
  // (Potential) END POSITION CALCULATION
  // - for current position, compute potential end position based on move value
  // ============================================================================
  std::vector<Position> vectorPossibleEndPos;

  // START move (zero moveValue) ///
  if (moveValue == 0) {
    Position possibleEndPos =
        Position(BoardLocation::TRACK, players[currentPlayer]->getStartField(),
                 currentPlayer);
    vectorPossibleEndPos.push_back(possibleEndPos);
  }

  /// Regular walking move (non-zero moveValue) ///
  else {
    size_t startFieldIdx = players[currentPlayer]->getStartField();
    /// FINISH area move (from FINISH to FINISH) ///
    if (marblePos.boardLocation == BoardLocation::FINISH) {
      int targetIndex = marblePos.index + moveValue;

      if (targetIndex < 0 || targetIndex > 3) {
        return std::nullopt;  // Invalid move, outside finish area bounds
      }

      // Check path for own marbles - no jumping over own marbles allowed
      int step = (moveValue > 0) ? 1 : -1;
      for (int checkIdx = marblePos.index + step;
           (step > 0 && checkIdx <= targetIndex) ||
           (step < 0 && checkIdx >= targetIndex);
           checkIdx += step) {
        Position checkPos(BoardLocation::FINISH, checkIdx, currentPlayer);
        auto occupant = isFieldOccupied(checkPos);
        // If any position along the path contains own marble, move is blocked
        if (occupant.has_value() && occupant->playerID == currentPlayer) {
          return std::nullopt;
        }
      }

      Position possibleEndPos =
          Position(BoardLocation::FINISH, targetIndex, currentPlayer);
      vectorPossibleEndPos.push_back(possibleEndPos);
    }

    /// TRACK area move (from TRACK to TRACK & from TRACK to FINISH) ///
    else {
      // Calculate end position index on track (with wrap-around for circular
      // track)
      int newIndex = static_cast<int>(marblePos.index) + moveValue;
      size_t endIndex;

      // Handle negative indices (moving backwards) - wrap around from 0 to 63
      if (newIndex < 0) {
        endIndex = static_cast<size_t>((64 + (newIndex % 64)) % 64);
      }
      // Handle positive indices (moving forwards) - wrap around from 63 to 0
      else {
        endIndex = static_cast<size_t>(newIndex % 64);
      }

      // End Index calculation done, now check for special cases preventing the
      // move

      // Special case: Start field is blocked (by marble sitting on start for
      // first time after home) If marble is on start field and start is
      // blocked, the field cannot be passed for track continuance or finish
      // entry

      // If move is made by blocked marble on start field it can move away to
      // the track only the game logic will then also unblock the start field
      // for future moves (handled in other functions)
      if (players[currentPlayer]->isStartBlocked() &&
          marblePos.index == startFieldIdx) {
        Position possibleEndPos =
            Position(BoardLocation::TRACK, endIndex, currentPlayer);
        vectorPossibleEndPos.push_back(possibleEndPos);
      }
      // Otherwise check for crossing blocked start fields
      else {
        bool crossesBlockedStart = false;
        bool crossesOurStart = false;
        // check all player start fields if they are blocked and if the move
        // crosses them
        for (size_t pID = 0; pID < 4; ++pID) {
          const auto& playerOpt = players[pID];
          if (!playerOpt.has_value()) {
            continue;  // Skip absent players
          }

          size_t anyStartIdx = playerOpt->getStartField();
          bool startBlocked = playerOpt->isStartBlocked();
          bool ourStart = (pID == currentPlayer);

          // Check if move path crosses a start field
          // Case 1: Moving forward (not wrapping around track)
          if ((moveValue > 0) && (marblePos.index < endIndex)) {
            // Simple range check: start < anyStartIdx <= end
            if (marblePos.index <= anyStartIdx && endIndex >= anyStartIdx) {
              if (startBlocked) {
                crossesBlockedStart = true;
              }
              if (ourStart && endIndex != startFieldIdx) {
                crossesOurStart = true;
              }
            }
          }
          // Case 2: Moving forward but wrapping around (e.g., from 62 to 2)
          else if ((moveValue > 0) && (marblePos.index > endIndex)) {
            // Check if blocked start is after marble position OR before end
            // position
            if (anyStartIdx >= marblePos.index || anyStartIdx <= endIndex) {
              if (startBlocked) {
                crossesBlockedStart = true;
              }
              if (ourStart && endIndex != startFieldIdx) {
                crossesOurStart = true;
              }
            }
          }
          // Case 3: Moving backward (not wrapping around track)
          else if ((moveValue < 0) && (marblePos.index > endIndex)) {
            // Simple range check: end <= anyStartIdx < start
            if (endIndex <= anyStartIdx && anyStartIdx <= marblePos.index) {
              if (startBlocked) {
                crossesBlockedStart = true;
              }
              if (ourStart && endIndex != startFieldIdx) {
                crossesOurStart = true;
              }
            }
          }
          // Case 4: Moving backward but wrapping around (e.g., from 2 to 62)
          else if ((moveValue < 0) && (marblePos.index < endIndex)) {
            // Check if blocked start is before marble position OR after end
            // position
            if (anyStartIdx <= marblePos.index || anyStartIdx >= endIndex) {
              if (startBlocked) {
                crossesBlockedStart = true;
              }
              if (ourStart && endIndex != startFieldIdx) {
                crossesOurStart = true;
              }
            }
          }
          // If we cross any blocked start, move is invalid
          if (crossesBlockedStart) {
            return std::nullopt;
          }
        }

        // Now check if move crosses OUR OWN start field (potential finish
        // entry) Only allow finish entry if our start is crossed
        if (crossesOurStart) {
          // Calculate how far into finish area the move would go
          // finishIndex is the potential position within finish area (valid
          // only: 0 to 3)
          size_t finishIndex;
          // Forward move crossing start
          if ((moveValue > 0)) {
            finishIndex = endIndex - startFieldIdx - 1;
          }
          // Backward move crossing start
          else {
            if (startFieldIdx == 0) {
              finishIndex = 64 - endIndex - 1;
            } else {
              finishIndex = startFieldIdx - endIndex - 1;
            }
          }

          if (finishIndex <= 3) {
            bool enterfinishAllowed = true;
            // Check in finish area for own marbles blocking the path
            for (int checkIdx = 0; checkIdx <= finishIndex; checkIdx++) {
              Position checkPos(BoardLocation::FINISH, checkIdx, currentPlayer);
              auto occupant = isFieldOccupied(checkPos);
              // If any position along the path contains own marble, move is
              // blocked
              if (occupant.has_value() && occupant->playerID == currentPlayer) {
                enterfinishAllowed = false;
                break;
              }
            }
            if (enterfinishAllowed) {
              // additionally add finish position as possible end position
              Position possibleEndPos =
                  Position(BoardLocation::FINISH, finishIndex, currentPlayer);
              vectorPossibleEndPos.push_back(possibleEndPos);
            }
          }
        }
        // anyways add the track end position as possible end position
        Position possibleEndPos =
            Position(BoardLocation::TRACK, endIndex, currentPlayer);
        vectorPossibleEndPos.push_back(possibleEndPos);
      }
    }
  }

  // Security checks
  if (vectorPossibleEndPos.empty()) {
    std::cerr << "Error: Should never reach here - no possible end positions "
                 "but did not leave earlier."
              << std::endl;
    return std::nullopt;  // No possible end positions computed -> invalid move
  } else if (vectorPossibleEndPos.size() > 2) {
    std::cerr << "Error: Should never reach here - more than 2 possible end "
                 "positions computed."
              << std::endl;
    return std::nullopt;  // More than 2 possible end positions computed ->
                          // invalid move
  }

  // ============================================================================
  // OCCUPATION TEST (after potential end position computed)
  // for all possible end positions, check if occupied and by whom
  // ============================================================================
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
      resultMoves = std::vector<std::pair<MarbleIdentifier, Position>>{};

  // For both possible end positions, check occupation
  for (const Position& possibleEndPos : vectorPossibleEndPos) {
    std::optional<MarbleIdentifier> occupyingMarble =
        isFieldOccupied(possibleEndPos);

    // End field unoccupied -> valid move
    if (!occupyingMarble.has_value()) {
      resultMoves->push_back(std::pair<MarbleIdentifier, Position>{
          MarbleIdentifier(
              currentPlayer,
              players[currentPlayer]->getMarbleIndexByPos(marblePos).value()),
          possibleEndPos});
    }
    // End field occupied
    else {
      // End field occupied by own marble -> invalid move
      if (occupyingMarble.value().playerID == currentPlayer) {
        continue;  // skip to next possible end position
      }
      // End field occupied by opponent marble -> valid move, send opponent home
      else {
        // Add: Moving marble to end pos
        resultMoves->push_back(std::pair<MarbleIdentifier, Position>{
            MarbleIdentifier(
                currentPlayer,
                players[currentPlayer]->getMarbleIndexByPos(marblePos).value()),
            possibleEndPos});
        // Add: opponent marble being sent home
        resultMoves->push_back(
            {occupyingMarble.value(),
             Position(BoardLocation::HOME,
                      occupyingMarble.value()
                          .marbleIdx,  // always send to own home pos Marble 0
                                       // to 0, 1 to 1, ...
                      occupyingMarble.value().playerID)});
      }
    }
  }
  // If no valid moves found -> invalid move
  if (!resultMoves.has_value() || resultMoves->empty()) {
    return std::nullopt;
  }
  return resultMoves;
}

// Check SWAP
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::checkSwapMove(const Position& marblePos) const {
  // Check all opponent marbles on track
  std::pair<MarbleIdentifier, Position> movingMarble(
      MarbleIdentifier(
          currentPlayer,
          players[currentPlayer]->getMarbleIndexByPos(marblePos).value()),
      marblePos);

  // Swap away if blocked on start not allowed
  if (players[currentPlayer]->getStartBlocked().has_value() &&
      players[currentPlayer]->getStartBlocked().value() ==
          movingMarble.first.marbleIdx) {
    return std::nullopt;  // Cannot swap if own marble is blocked on start
  }

  std::vector<std::pair<MarbleIdentifier, Position>> swapMoves;

  for (size_t pID = 0; pID < 4; ++pID) {
    if (pID == currentPlayer) {
      continue;  // Skip own player
    }
    const auto& opponentOpt = players[pID];
    if (!opponentOpt.has_value()) {
      continue;  // Skip absent players
    }
    // For each marble of opponent
    for (size_t mIdx = 0; mIdx < 4; ++mIdx) {
      const Position& opponentMarblePos = opponentOpt->getMarblePosition(mIdx);
      // Check if on TRACK and if not blocked on start
      if (opponentMarblePos.boardLocation == BoardLocation::TRACK) {
        if (opponentOpt->getStartBlocked().has_value() &&
            opponentOpt->getStartBlocked().value() == mIdx) {
          continue;  // Skip blocked opponent marble
        }
        // Valid swap candidate
        std::pair<MarbleIdentifier, Position> opponentMarble(
            MarbleIdentifier(pID, mIdx), opponentMarblePos);
        swapMoves.push_back(
            {movingMarble.first, opponentMarble.second});  // moving marble
        swapMoves.push_back(
            {opponentMarble.first,
             movingMarble.second});  // opponent marble to moving marble pos
      }
    }
  }
  // If no valid swap moves found -> invalid move
  if (swapMoves.empty()) {
    return std::nullopt;
  }
  // Return all possible swap moves
  return swapMoves;
}

// Check SEVEN
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::checkSevenMove(const Position& marblePos, int moveValue) const {
  // ============================================================================
  // (Potential) END POSITION CALCULATION
  // - for current position, compute potential end position based on move value
  // ============================================================================
  std::vector<std::pair<MarbleIdentifier, Position>> walkingOptions;

  BraendiDog::MarbleIdentifier movingMarble(
      currentPlayer,
      players[currentPlayer]->getMarbleIndexByPos(marblePos).value());

  std::vector<MarbleIdentifier> sentHomeMarbles =
      std::vector<MarbleIdentifier>();

  /// Regular walking move for all moveValue parts upto moveValue ///
  const auto& playerOpt = players[currentPlayer];
  size_t ourStartIdx = playerOpt->getStartField();
  auto ourStartBlocked = playerOpt->getStartBlocked();
  bool crossesOurStart = false;

  for (int mvPart = 1; mvPart <= moveValue; ++mvPart) {
    /// FINISH area move (from FINISH to FINISH) ///
    if (marblePos.boardLocation == BoardLocation::FINISH) {
      int targetIndex = marblePos.index + mvPart;

      if (targetIndex > 3) {
        break;  // Invalid move, outside finish area bounds
      }

      if (isFieldOccupied(
              Position(BoardLocation::FINISH, targetIndex, currentPlayer))
              .has_value()) {
        break;  // Blocked by own marble
      }

      Position possibleEndPos =
          Position(BoardLocation::FINISH, targetIndex, currentPlayer);
      walkingOptions.push_back({movingMarble, possibleEndPos});
    }

    /// TRACK area move (from TRACK to TRACK & from TRACK to FINISH) ///
    else {
      // Calculate end position index on track (with wrap-around for circular
      // track)
      int newIndex = static_cast<int>(marblePos.index) + mvPart;
      size_t endIndex;

      // Handle negative indices (moving backwards) - wrap around from 0 to 63
      if (newIndex < 0) {
        endIndex = static_cast<size_t>((64 + (newIndex % 64)) % 64);
      }
      // Handle positive indices (moving forwards) - wrap around from 63 to 0
      else {
        endIndex = static_cast<size_t>(newIndex % 64);
      }

      // End Index calculation done, now check for special cases preventing the
      // move

      // If move did not already cross our own start, check if it does now
      if (!crossesOurStart) {
        // Check if move path crosses our start field
        // Case 1: Moving forward (not wrapping around track)
        if (marblePos.index < endIndex) {
          // Simple range check: start <= ourStartIdx < end
          if (marblePos.index <= ourStartIdx && endIndex > ourStartIdx) {
            crossesOurStart = true;
          }
        }
        // Case 2: Moving forward but wrapping around (e.g., from 62 to 2)
        else if (marblePos.index > endIndex) {
          // Check if blocked start is after marble position OR before end
          // position
          if (ourStartIdx >= marblePos.index || ourStartIdx < endIndex) {
            crossesOurStart = true;
          }
        }
      }

      // Check if occupied by opponent marble
      auto occupant = isFieldOccupied(
          Position(BoardLocation::TRACK, endIndex, currentPlayer));

      // FIRST: Check if trying to pass a start-blocked marble (BLOCKING - stops
      // all walking)
      if (occupant.has_value() && occupant->playerID == currentPlayer) {
        if (ourStartBlocked.has_value() &&
            ourStartBlocked.value() == occupant->marbleIdx) {
          break;  // Can't pass start-blocked marble - stop advancing
        }
      }

      if (occupant.has_value() && occupant->playerID != currentPlayer) {
        // access opponent player
        const auto& opponentPlayerOpt = players[occupant->playerID];
        // Access get start blocked info
        if (opponentPlayerOpt->getStartBlocked().has_value() &&
            opponentPlayerOpt->getStartBlocked().value() ==
                occupant->marbleIdx) {
          // Occupant is blocked on start -> cannot be sent home
          break;  // stop advancing further in this walking option
        }
        // add opponent MarbleIdentifier to sentHomeMarbles
        sentHomeMarbles.push_back(occupant.value());
      }

      // If we cross our start, check for finish entry
      bool enterfinishAllowed = false;
      size_t finishIndex;
      if (crossesOurStart) {
        // Calculate how far into finish area the move would go
        // finishIndex is the potential position within finish area (valid
        // only: 0 to 3)
        finishIndex = endIndex - ourStartIdx - 1;

        if (finishIndex <= 3) {
          enterfinishAllowed = true;
        }
        if (ourStartBlocked.has_value() &&
            ourStartBlocked.value() == movingMarble.marbleIdx) {
          enterfinishAllowed = false;  // cannot enter finish if blocked on
                                       // start
        }
      }

      // Now add finish check as well if possible
      if (enterfinishAllowed) {
        // Check in finish area for own marbles blocking the path
        auto occupantFinish = isFieldOccupied(
            Position(BoardLocation::FINISH, finishIndex, currentPlayer));

        if (!occupantFinish.has_value()) {
          // Only if unoccupied also add finish position as possible end
          // position
          Position possibleEndPos =
              Position(BoardLocation::FINISH, finishIndex, currentPlayer);
          walkingOptions.push_back({movingMarble, possibleEndPos});

          // Add sent home opponent marble as well
          for (const MarbleIdentifier& oppMarble : sentHomeMarbles) {
            // access opponent player
            const auto& opponentPlayerOpt = players[oppMarble.playerID];
            if (opponentPlayerOpt->getMarblePosition(oppMarble.marbleIdx)
                    .index > ourStartIdx) {
              continue;  // skip adding opponent marble if it is after our start
                         // (for finish entry)
            }
            walkingOptions.push_back(
                {oppMarble, Position(BoardLocation::HOME, oppMarble.marbleIdx,
                                     oppMarble.playerID)});
          }
        }
      }

      // Add track position ONLY if not occupied by own marble
      if (occupant.has_value() && occupant->playerID == currentPlayer) {
        continue;  // skip adding track position if occupied by own marble
      }

      // Add track end position as possible end position
      Position possibleEndPos =
          Position(BoardLocation::TRACK, endIndex, currentPlayer);
      walkingOptions.push_back({movingMarble, possibleEndPos});

      // Add sent home opponent marble as well
      for (const MarbleIdentifier& oppMarble : sentHomeMarbles) {
        walkingOptions.push_back(
            {oppMarble, Position(BoardLocation::HOME, oppMarble.marbleIdx,
                                 oppMarble.playerID)});
      }
    }
  }

  // If no valid moves found -> invalid move
  if (walkingOptions.empty()) {
    return std::nullopt;
  }
  std::cout << "SEVEN move options computed: " << walkingOptions.size()
            << " parts." << std::endl;
  return walkingOptions;
}

// Check JOKER
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::checkJokerMove() const {
  return std::nullopt;  // TODO: implement
}

// Validate Move
std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
GameState::validateMove(const Card& card, const Position& marblePos,
                        const std::pair<MoveType, int>& moveRule,
                        bool sevenCall) const {
  MoveType moveType = moveRule.first;
  int moveValue = moveRule.second;

  // Switch statement for location
  switch (marblePos.boardLocation) {
    case BoardLocation::HOME:
      switch (moveType) {
        case MoveType::START:
          return checkStartMove(marblePos);
          break;
        default:
          return std::nullopt;  // Invalid move
      }
      break;
    case BoardLocation::FINISH:
      switch (moveType) {
        case MoveType::SIMPLE:
          return checkSimpleMove(marblePos, moveValue);
          break;
        case MoveType::SEVEN:
          if (sevenCall) {
            return checkSevenMove(
                marblePos, moveValue);  // Adjusted walking logic for SEVEN call
          } else {
            return std::nullopt;  // Ignore SEVEN card in non sevenCall context
          }
          break;
        default:
          return std::nullopt;  // Invalid move
      }
      break;
    case BoardLocation::TRACK:
      switch (moveType) {
        case MoveType::SIMPLE:
          return checkSimpleMove(marblePos, moveValue);
          break;
        case MoveType::SEVEN:
          if (sevenCall) {
            return checkSevenMove(
                marblePos, moveValue);  // Adjusted walking logic for SEVEN call
          } else {
            return std::nullopt;  // Ignore SEVEN card in non sevenCall context
          }
          break;
        case MoveType::SWAP:
          return checkSwapMove(marblePos);
          break;
        default:
          return std::nullopt;  // Invalid move
      }
      break;
    default:
      std::cerr << "Invalid board location in validateMove" << std::endl;
      return std::nullopt;  // Invalid location
  }
}

// Compute all legal plays for the current player given their hand and marble
// positions.
std::vector<BraendiDog::Move> GameState::computeLegalMoves(
    std::optional<std::array<size_t, 3>> Special, bool sevenCall) const {
  std::vector<BraendiDog::Move> legalMoves;

  bool jokerCall = false;

  // Access current players hand and marbles
  const Player& currentPlayerObj = players[currentPlayer].value();
  const std::array<Position, 4>& marbles = currentPlayerObj.getMarbles();

  // Make Function work for Special Cards as well
  std::vector<size_t> hand;
  if (Special.has_value()) {
    // Joker or Seven Computation
    hand = std::vector<size_t>{Special.value()[0]};
    if (deck[Special.value()[2]].getRank() == Rank::JOKER) {
      jokerCall = true;
    }
  } else {
    hand = currentPlayerObj.getHand();
  }

  // For each card in hand
  for (size_t handIndex = 0; handIndex < hand.size(); ++handIndex) {
    size_t cardID = hand[handIndex];
    const Card& card = deck[cardID];

    size_t toSetHandIndex;
    size_t toSetCardID;
    if (jokerCall || sevenCall) {
      // Use synthetic card for joker/seven call
      toSetHandIndex = Special.value()[1];
      toSetCardID = Special.value()[2];
    } else {
      toSetHandIndex = handIndex;
      toSetCardID = cardID;
    }

    // For each move rule of the card
    for (const auto& moveRule : card.getMoveRules()) {
      MoveType effectiveMoveType = moveRule.first;
      int effectiveMoveValue = moveRule.second;
      // If seven - skip certain moverules
      if (sevenCall) {
        if ((moveRule.first == MoveType::SIMPLE &&
             (moveRule.second > 7 || moveRule.second < 1)) ||
            (moveRule.first != MoveType::SIMPLE &&
             moveRule.first != MoveType::SEVEN)) {
          continue;  // skip non-simple/seven move rules for seven card
        }
        if (moveRule.first == MoveType::SIMPLE) {
          // treat synthetic SIMPLE as SEVEN, moveValue stays as defined in
          // synthetic Card
          effectiveMoveType = MoveType::SEVEN;
        }
      }

      // For each marble of the current player
      for (size_t mIdx = 0; mIdx < marbles.size(); ++mIdx) {
        const Position& marblePos = marbles[mIdx];

        // validateMove
        auto movedMarbles = validateMove(
            card, marblePos,
            std::make_pair(effectiveMoveType, effectiveMoveValue), sevenCall);
        // Add to legal moves if valid
        if (movedMarbles.has_value()) {
          // for START only one movement option for
          // card+cardRule+marble combination
          // for SIMPLE upto three movement options possible
          // (finish/track+capture)
          if (effectiveMoveType == MoveType::START ||
              effectiveMoveType == MoveType::SIMPLE) {
            const auto& movements = movedMarbles.value();

            if (movements.size() >= 2 &&
                movements[0].first.marbleIdx == movements[1].first.marbleIdx &&
                movements[0].first.playerID == movements[1].first.playerID &&
                movements[0].second.boardLocation !=
                    movements[1].second.boardLocation) {
              // Create finish option (never has captures)
              std::vector<std::pair<MarbleIdentifier, Position>> finishOption;
              finishOption.push_back(movements[0]);
              legalMoves.push_back(
                  Move(toSetCardID, toSetHandIndex, finishOption));

              // Create track option (may have capture at index 2)
              std::vector<std::pair<MarbleIdentifier, Position>> trackOption;
              trackOption.push_back(movements[1]);
              if (movements.size() >= 3 &&
                  movements[2].first.playerID != currentPlayer &&
                  movements[2].second.boardLocation == BoardLocation::HOME) {
                trackOption.push_back(movements[2]);
              }
              legalMoves.push_back(
                  Move(toSetCardID, toSetHandIndex, trackOption));
            } else {
              Move move(toSetCardID, toSetHandIndex, movements);
              legalMoves.push_back(move);
            }
          }
          // for SWAP multiple movement options possible (depending on opponent
          // marbles)
          else if (effectiveMoveType == MoveType::SWAP) {
            // one option will have 2 marbles moved (own current Player marble +
            // opponent marble)
            for (size_t i = 0; i < movedMarbles->size(); i += 2) {
              std::vector<std::pair<MarbleIdentifier, Position>> swapMoveVec;
              swapMoveVec.push_back(movedMarbles->at(
                  i));  // moving marble (all playerID = currentPlayer)
              swapMoveVec.push_back(movedMarbles->at(i + 1));  //
              Move move(toSetCardID, toSetHandIndex, swapMoveVec);
              legalMoves.push_back(move);
            }
          }
          // for SEVEN call multiple movement options possible
          // split into moves of own marbles moved (if applicable with opponent
          // sent home)
          else if (effectiveMoveType == MoveType::SEVEN) {
            // one option will have 1 or more marbles moved (own current Player
            // marble + any optional opponent marble)
            // at own marble a new move starts
            for (size_t i = 0; i < movedMarbles->size();) {
              std::vector<std::pair<MarbleIdentifier, Position>> sevenMoveVec;
              // Add own marble move
              sevenMoveVec.push_back(movedMarbles->at(i));
              i++;
              // Add any opponent marbles sent home
              while (i < movedMarbles->size() &&
                     movedMarbles->at(i).first.playerID != currentPlayer) {
                sevenMoveVec.push_back(movedMarbles->at(i));
                i++;
              }
              Move move(toSetCardID, toSetHandIndex, sevenMoveVec);
              legalMoves.push_back(move);
            }
          }

          /// For efficiency skip other marble validations if possible

          // if MoveType == START and valid duplicate for other marbles (where
          // loc/type comapatible)
          if (effectiveMoveType == MoveType::START) {
            // duplicate moved Marbles with other marbles of same location
            mIdx++;
            while (mIdx < marbles.size()) {
              if (players[currentPlayer]
                      ->getMarblePosition(mIdx)
                      .boardLocation != BraendiDog::BoardLocation::HOME) {
                mIdx++;
                continue;
              }

              std::vector<std::pair<MarbleIdentifier, Position>> startMoveVec;
              auto currMovedMarbles = movedMarbles->at(0);
              currMovedMarbles.first.marbleIdx = mIdx;  // update marbleIdx
              startMoveVec.push_back(
                  currMovedMarbles);  // moving marble (all playerID =
                                      // currentPlayer)
              Move move(toSetCardID, toSetHandIndex, startMoveVec);
              legalMoves.push_back(move);

              mIdx++;
            }
            break;
          }
          // if MoveType == SWAP and valid duplicate for other marbles (where
          // loc/type comapatible)
          if (effectiveMoveType == MoveType::SWAP) {
            // duplicate moved Marbles with other marbles of same location
            mIdx++;
            while (mIdx < marbles.size()) {
              if (players[currentPlayer]
                      ->getMarblePosition(mIdx)
                      .boardLocation != BraendiDog::BoardLocation::TRACK) {
                mIdx++;
                continue;
              }
              if (players[currentPlayer]->getStartBlocked().has_value() &&
                  players[currentPlayer]->getStartBlocked().value() == mIdx) {
                mIdx++;
                continue;  // Skip blocked marble
              }
              for (size_t i = 0; i < movedMarbles->size(); i += 2) {
                std::vector<std::pair<MarbleIdentifier, Position>> swapMoveVec;
                auto currMovedMarbles = movedMarbles->at(i);
                currMovedMarbles.first.marbleIdx = mIdx;  // update marbleIdx
                swapMoveVec.push_back(
                    currMovedMarbles);  // moving marble (all playerID =
                                        // currentPlayer)
                auto opponentMovement = movedMarbles->at(i + 1);
                opponentMovement.second =
                    players[currentPlayer]->getMarblePosition(mIdx);
                swapMoveVec.push_back(
                    opponentMovement);  // swapped marble (opponent)
                Move move(toSetCardID, toSetHandIndex, swapMoveVec);
                legalMoves.push_back(move);
              }
              mIdx++;
            }
            break;
          }
        }
      }
    }
  }
  std::cout << "Computed " << legalMoves.size() << " legal moves for player "
            << currentPlayer << std::endl;
  // cout legal moves for debugging
  for (const auto& move : legalMoves) {
    std::cout << "Move: CardID " << move.getCardID() << ", HandIndex "
              << move.getHandIndex() << ", Movements: ";
    for (const auto& movement : move.getMovements()) {
      std::cout << "[Player " << movement.first.playerID << " Marble "
                << movement.first.marbleIdx << " -> ("
                << static_cast<int>(movement.second.boardLocation) << ", "
                << movement.second.index << ")] ";
    }
    std::cout << std::endl;
  }
  // return all legal moves
  return legalMoves;
}

bool GameState::validJokerFold() const {
  // Valid Joker fold only if no marbles are available for swapping
  // AND each marble is instantly blocked by own marble, blocked start or inner
  // most finish
  const Player& currentPlayerObj = players[currentPlayer].value();
  const std::array<Position, 4>& marbles = currentPlayerObj.getMarbles();

  // If no Joker in hand, valid fold
  if (!currentPlayerObj.hasJokerInHand()) {
    return true;
  }

  std::vector<size_t> blockedStarts;
  for (size_t pIdx = 0; pIdx < 4; ++pIdx) {
    const auto& playerOpt = players[pIdx];
    if (!playerOpt.has_value() && pIdx == currentPlayer) {
      continue;  // Skip absent players and self
    }
    if (playerOpt->isStartBlocked()) {
      blockedStarts.push_back(playerOpt->getStartField());
    }
  }

  for (size_t mIdx = 0; mIdx < marbles.size(); ++mIdx) {
    const Position& marblePos = marbles[mIdx];

    // Only possible if all marbles outside HOME
    if (marblePos.boardLocation == BoardLocation::HOME) {
      return false;  // HOME marbles prevent fold because either they can start
                     // or there is a marble on start that can move
    }

    // Check SWAP move possibility
    if (marblePos.boardLocation == BoardLocation::TRACK) {
      auto swapMoves = checkSwapMove(marblePos);
      if (swapMoves.has_value()) {
        return false;  // Valid swap move found, cannot fold
      }
    }

    // If on TRACK or FINISH, check if blocked instantly
    // by own marble, blocked start or inner most finish

    // Check inner most finish
    if (marblePos.boardLocation == BoardLocation::FINISH &&
        marblePos.index == 3) {
      continue;  // Inner most finish position blocks further movement
    }

    // Check own marble blocking or blocked start
    Position nextPos;
    size_t nextIndex;

    // TRACK (wrap case)
    if (marblePos.boardLocation == BoardLocation::TRACK &&
        marblePos.index == 63) {
      nextIndex = 0;
    }
    // every other case (FINISH or TRACK non-wrap)
    else {
      nextIndex = marblePos.index + 1;
    }
    nextPos = Position(marblePos.boardLocation, nextIndex, currentPlayer);

    // Check for marble blocking
    auto occupant = isFieldOccupied(nextPos);

    // If no occupant found walking ahead is possible
    if (!occupant.has_value()) {
      return false;  // Free field ahead, cannot fold (with Joker)
    }
    // If occupant is own marble - valid for fold
    if (occupant.has_value() && occupant->playerID == currentPlayer) {
      continue;
    }
    // If occupant is start blocked marble - valid for fold
    if (marblePos.boardLocation == BoardLocation::TRACK &&
        occupant.has_value()) {
      if (blockedStarts.end() != std::find(blockedStarts.begin(),
                                           blockedStarts.end(),
                                           nextPos.index)) {
        continue;
      }
    }
    // If we reach here, this marble didn't pass any valid "blocked" check
    return false;  // Movement possible, cannot fold
  }
  return true;
}

bool GameState::validSevenFold() const {
  // TODO: as long as Seven move is not implemented, always return true
  // return true;

  // count unblocked fields if >= 7 then cannot fold
  const Player& currentPlayerObj = players[currentPlayer].value();
  const std::array<Position, 4>& marbles = currentPlayerObj.getMarbles();

  // If no Seven in hand, valid fold
  if (!currentPlayerObj.hasCardInHand(7)) {
    return true;
  }

  std::vector<size_t> blockedStarts;
  for (size_t pIdx = 0; pIdx < 4; ++pIdx) {
    const auto& playerOpt = players[pIdx];
    if (!playerOpt.has_value()) {
      continue;  // Skip absent players and self
    }
    if (playerOpt->isStartBlocked()) {
      blockedStarts.push_back(playerOpt->getStartField());
    }
  }

  size_t unblockedFieldCount = 0;
  for (size_t mIdx = 0; mIdx < marbles.size(); ++mIdx) {
    const Position& marblePos = marbles[mIdx];

    // Marbles to exclude from possible walking with Seven
    // - HOME marbles
    // - Inner most finish position marble
    if (marblePos.boardLocation == BoardLocation::HOME) {
      continue;  // HOME marbles cannot walk with Seven, skip
    }
    if (marblePos.boardLocation == BoardLocation::FINISH &&
        marblePos.index == 3) {
      continue;  // Inner most finish position blocks further movement
    }

    Position nextPos;
    size_t nextIndex = marblePos.index;
    size_t ownMarblesSkipped = 0;
    // Check unblocked fields ahead
    for (int step = 0; step < 8; ++step) {
      // TRACK (wrap case)
      if (marblePos.boardLocation == BoardLocation::TRACK && nextIndex == 63) {
        nextIndex = 0;
      }
      // every other case (FINISH or TRACK non-wrap)
      else {
        nextIndex++;
      }
      if (marblePos.boardLocation == BoardLocation::FINISH && nextIndex == 4) {
        unblockedFieldCount -= ownMarblesSkipped;
        break;  // Inner most finish position blocks further movement
      }

      nextPos = Position(marblePos.boardLocation, nextIndex, currentPlayer);

      // Check blocked start
      if (nextPos.boardLocation == BoardLocation::TRACK) {
        if (blockedStarts.end() != std::find(blockedStarts.begin(),
                                             blockedStarts.end(),
                                             nextPos.index)) {
          unblockedFieldCount -= ownMarblesSkipped;
          break;  // Blocked start field blocks further movement
        }
      }

      auto occupant = isFieldOccupied(nextPos);

      // If occupant is own marble update skipped count and continue
      if (occupant.has_value() && occupant->playerID == currentPlayer) {
        ownMarblesSkipped++;
      }

      // If we reach here, field is unblocked
      unblockedFieldCount++;

      if (unblockedFieldCount >= 7) {
        return false;  // Enough unblocked fields found, cannot fold
      }
    }

    if (unblockedFieldCount >= 7) {
      return false;  // Enough unblocked fields found, cannot fold
    }
  }

  if (unblockedFieldCount >= 7) {
    return false;  // Enough unblocked fields found, cannot fold
  }
  return true;
}

std::pair<bool, bool> GameState::hasSpecialMoves() const {
  bool hasJokerMoves = !validJokerFold();
  bool hasSevenMoves = !validSevenFold();
  return std::make_pair(hasJokerMoves, hasSevenMoves);
}

bool GameState::hasLegalMoves() const {
  auto legalMoves = computeLegalMoves();
  bool hasNormalMoves = !legalMoves.empty();

  auto [hasJokerMoves, hasSevenMoves] = hasSpecialMoves();
  std::cout << "Player " << currentPlayer
            << " has legal moves: " << (hasNormalMoves ? "Yes" : "No")
            << ", Joker moves: " << (hasJokerMoves ? "Yes" : "No")
            << ", Seven moves: " << (hasSevenMoves ? "Yes" : "No") << std::endl;
  return hasNormalMoves || hasJokerMoves || hasSevenMoves;
}

void GameState::applyTempSevenMove(const Move& move) {
  // Apply each movement in the move
  for (const auto& movement : move.getMovements()) {
    const auto& marbleId = movement.first;
    const auto& newPos = movement.second;

    // Update marble position
    if (players[marbleId.playerID].has_value()) {
      players[marbleId.playerID]->setMarblePosition(marbleId.marbleIdx, newPos);

      // Unblock if start-blocked marble moved
      if (players[marbleId.playerID]->isStartBlocked() &&
          players[marbleId.playerID]->getStartBlocked().value() ==
              marbleId.marbleIdx) {
        if (marbleId.playerID != currentPlayer) {
          std::cerr << "Player " << marbleId.playerID
                    << "'s start-blocked marble " << marbleId.marbleIdx
                    << " has moved and is now unblocked -> This should not be "
                       "allowed with Seven Move."
                    << std::endl;
        }
        players[marbleId.playerID]->resetStartBlocked();
      }
    }
  }
}

/// Server Game State Manipulation Methods ///

// Server Validate Turn
bool GameState::isValidTurn(const BraendiDog::Move& move) const {
  // FOLD
  // function called without passed move
  if (move.getMovements().empty()) {
    if (hasLegalMoves()) {
      std::cerr
          << "Error: executeFold called when legal moves exist for player "
          << currentPlayer << std::endl;
      return false;
    }
    return true;
  }

  // MOVE
  // function called with passed move
  // translate passed move to arguments for validateMove
  size_t cardID = move.getCardID();
  size_t handIndex = move.getHandIndex();
  const Card& card = deck[cardID];
  const auto& movements = move.getMovements();

  // actively moved marble is the first in movements
  MarbleIdentifier activeMarbleID = movements[0].first;

  // Current player validation - must be current player's turn
  if (activeMarbleID.playerID != currentPlayer) {
    std::cerr << "Error: executeMove called for non-current player "
              << activeMarbleID.playerID << " (current player is "
              << currentPlayer << ")" << std::endl;
    return false;
  }

  if (card.getRank() == Rank::JOKER) {
    // Joker move validation not yet implemented
    // Move follows same logic as normal move so lets assume joker moves are not
    // cheated ;)
    // TODO: implement proper joker move validation
    return true;
  }
  if (card.getRank() == Rank::SEVEN) {
    // Seven move validation not yet implemented
    // Move follows same logic as normal move so lets assume seven moves are not
    // cheated ;)
    // TODO: implement proper seven move validation
    return true;
  }

  Position activeMarblePos =
      players[activeMarbleID.playerID]->getMarblePosition(
          activeMarbleID.marbleIdx);

  // check all moverules of the card for one valid match
  for (const auto& moveRule : card.getMoveRules()) {
    auto validatedMovements = validateMove(card, activeMarblePos, moveRule);

    // if valid check if movements match
    if (validatedMovements.has_value()) {
      const auto& valMoves = validatedMovements.value();

      // For multi-option moves (SWAP, finish/track alternatives),
      // client sends one choice but validateMove returns all options
      // Skip size check in these cases - matching logic below will verify
      if (movements.size() < valMoves.size()) {
        // Multi-option case - continue to matching logic
      } else if (valMoves.size() != movements.size()) {
        std::cerr << "Error: Movement count mismatch for player "
                  << currentPlayer << " - expected " << valMoves.size()
                  << " movements, got " << movements.size() << std::endl;
        continue;  // try next moverule
      }

      // Check if movements match
      bool allMatch = true;

      if (movements.size() < valMoves.size()) {
        // Multi-option case: verify client's movements are a valid subset of
        // valMoves Client picks one option (e.g., finish or track), we verify
        // it's valid
        for (const auto& clientMove : movements) {
          bool matchFound = false;
          for (const auto& validMove : valMoves) {
            if (validMove.first.playerID == clientMove.first.playerID &&
                validMove.first.marbleIdx == clientMove.first.marbleIdx &&
                validMove.second.boardLocation ==
                    clientMove.second.boardLocation &&
                validMove.second.index == clientMove.second.index) {
              matchFound = true;
              break;
            }
          }
          if (!matchFound) {
            allMatch = false;
            std::cerr << "Error: Movement mismatch for player " << currentPlayer
                      << " - marble " << clientMove.first.marbleIdx
                      << " move to position ("
                      << static_cast<int>(clientMove.second.boardLocation)
                      << ", " << clientMove.second.index
                      << ") not found in valid options" << std::endl;
            break;
          }
        }
      } else {
        // Normal case: all server movements must be in client's move
        for (const auto& valMove : valMoves) {
          bool matchFound = false;
          for (const auto& moveMovement : movements) {
            if (valMove.first.playerID == moveMovement.first.playerID &&
                valMove.first.marbleIdx == moveMovement.first.marbleIdx &&
                valMove.second.boardLocation ==
                    moveMovement.second.boardLocation &&
                valMove.second.index == moveMovement.second.index) {
              matchFound = true;
              break;
            }
          }
          if (!matchFound) {
            allMatch = false;
            std::cerr << "Error: Movement mismatch for player " << currentPlayer
                      << " - marble " << valMove.first.marbleIdx
                      << " expected at position ("
                      << static_cast<int>(valMove.second.boardLocation) << ", "
                      << valMove.second.index << ")" << std::endl;
            break;
          }
        }
      }

      if (allMatch) {
        return true;  // valid move found
      }
    }
  }

  // no valid moverule match found
  std::cerr << "Error: executeMove called with invalid move for player "
            << activeMarbleID.playerID << " - no matching move rule found"
            << std::endl;
  return false;
}

// Turn end check and procedures
// Returns pair<bool,bool> indicating (roundEnded,gameEnded)
std::pair<bool, bool> GameState::endTurn() {
  bool gameEnded = checkGameEnd();
  bool roundEnded = checkRoundEnd();

  // Check if game has ended
  if (gameEnded) {
    // Add remaining active player to leaderboard
    size_t remainingPlayer =
        getActivePlayerIndices()[0];  // only one remaining player
    addLeaderBoardUnfinished(remainingPlayer);
  }
  // If round ended but game not ended -> update for next round
  else if (roundEnded) {
    // Update round start player
    updateRoundStartPlayer();
    // Update round card count
    updateRoundCardCount();

    // Reset all players to active in round (if active in game)
    for (auto& playerOpt : players) {
      if (playerOpt.has_value() && playerOpt->isActiveInGame()) {
        playerOpt->setActiveInRound(true);
      }
    }
  } else {
    // Update current player to next active in round
    updateCurrentPlayer();
  }

  return std::make_pair(gameEnded, roundEnded);
}

// Execute Fold
void GameState::executeFold() {
  // Remove all cards from player's hand
  players[currentPlayer]->setHand(std::vector<size_t>());

  // Check if players round ended
  // Update player status accordingly
  if (players[currentPlayer]->getHand().empty()) {
    players[currentPlayer]->setActiveInRound(false);
  }
}

// Execute Move
bool GameState::executeMove(BraendiDog::Move move) {
  // Update marble positions
  for (const auto& movement : move.getMovements()) {
    size_t pID = movement.first.playerID;
    size_t mIdx = movement.first.marbleIdx;
    const Position& newPos = movement.second;
    bool moveFromHome = players[pID]->getMarblePosition(mIdx).boardLocation ==
                        BoardLocation::HOME;

    // Update marble position
    players[pID]->setMarblePosition(mIdx, newPos);

    // Update start blockage status
    // Check if current player marble moved from start field for the first time
    if (pID == currentPlayer) {
      // Unblock start blocking marble moved
      std::optional<size_t> startBlocked =
          players[currentPlayer]->getStartBlocked();
      if (startBlocked.has_value() && startBlocked.value() == mIdx) {
        players[currentPlayer]->resetStartBlocked();
      }
      // Block start field if marble moves to start field from home
      else if (moveFromHome) {
        players[currentPlayer]->setStartBlocked(mIdx);
      }
    }
  }

  // Remove card from player's hand
  // Update last played card
  size_t handIndex = move.getHandIndex();
  lastPlayedCard = players[currentPlayer]->popCardFromHand(handIndex);

  // Check if players round ended
  // Update player status accordingly
  if (players[currentPlayer]->getHand().empty()) {
    players[currentPlayer]->setActiveInRound(false);
  }

  // Check if player has finished
  // Update status & leaderboard accordingly
  if (players[currentPlayer]->checkFinished()) {
    players[currentPlayer]->setHand(
        std::vector<size_t>());  // remove any remaining cards
    players[currentPlayer]->setActiveInRound(false);
    players[currentPlayer]->setActiveInGame(false);
    addLeaderBoardFinished(currentPlayer);
    return true;
  }
  // Move executed successfully, player not finished
  return false;
}

}  // namespace BraendiDog
