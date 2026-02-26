#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "client/client.hpp"
#include "shared/game.hpp"
#include "shared/game_types.hpp"

class MovePhaseController {
 public:
  explicit MovePhaseController(Client* client, size_t myPlayerIndex,
                               BraendiDog::GameState& gameState);

  // UI actions (call when the user clicks)
  /**
   * @brief Handle the event when a card in the player's hand is clicked.
   * @param handIndex The index of the clicked card in the player's hand.
   */
  void onCardClicked(int handIndex);
  /**
   * @brief Handle the event when a board position is clicked.
   * @param pos The position on the board that was clicked.
   */
  void onBoardPositionClicked(const BraendiDog::Position& pos);

  // helpers / getters
  /**
   * @brief Get the index of the currently selected card in hand.
   * @return The index of the selected card, or -1 if no card is selected.
   */
  int getSelectedHandIndex() const;
  /**
   * @brief Get the currently selected marble identifier.
   * @return An optional containing the selected marble identifier, or
   * std::nullopt if none is selected.
   */
  std::optional<BraendiDog::MarbleIdentifier> getSelectedMarble() const;
  /**
   * @brief Get the possible destination positions based on the current
   * selection.
   * @return A vector of possible destination positions.
   */
  std::vector<BraendiDog::Position> getPossibleDestinations() const;
  /**
   * @brief Sets the rank for a Joker card
   * @param rank The rank value (1-13 for A-K)
   */
  void setJokerRank(int rank, size_t jokerHandIndex);

  /**
   * @brief Gets the currently selected Joker rank
   * @return The selected rank, or -1 if not set
   */
  int getJokerRank() const;

  /**
   * @brief Clears the selected Joker rank
   */
  void clearJokerRank();

  // Setter for legal moves
  /**
   * @brief Set the legal moves available for the current turn.
   * @param moves A vector of legal moves.
   */
  void setLegalMoves(const std::vector<BraendiDog::Move> moves);

  /**
   * @brief Set the currently built Seven move.
   * @param move The Seven move being built.
   */
  void setBuiltSevenMove(BraendiDog::Move& move);

  /**
   * @brief Clear the currently built Seven move.
   */
  void clearBuiltSevenMove();

  /**
   * @brief Get the currently built Seven move.
   * @param move Reference to store the built Seven move.
   */
  void getBuiltSevenMove(BraendiDog::Move& move) const;

  /**
   * @brief Set the possible Seven moves.
   * @param moves A vector of possible Seven moves.
   */
  void setSevenMoves(const std::vector<BraendiDog::Move> moves);

  /**
   * @brief Clear the possible Seven moves.
   */
  void clearSevenMoves();

  /**
   * @brief Get the possible Seven moves.
   * @param moves Reference to store the possible Seven moves.
   */
  void getSevenMoves(std::vector<BraendiDog::Move>& moves) const;

  /**
   * @brief Get the temporary game state used during Seven move building
   * @return Optional containing temp state, or nullopt if not building Seven
   */
  std::optional<BraendiDog::GameState> getSevenTempState() const;

  /**
   * @brief Get the total Seven move value built so far
   * @return Value from 0-7
   */
  size_t getTotalSevenMoveValue() const;

  // Filtering of legal moves based on selection
  /**
   * @brief Filter the legal moves based on the current selection - Card only.
   */
  void filterByCard(int handIndex);
  /**
   * @brief Filter the legal moves based on the current selection - Marble only.
   * @return true if marble has legal moves, false otherwise
   */
  bool filterByMarble(const BraendiDog::MarbleIdentifier& marbleId);
  /**
   * @brief Find a matching move for a given destination position.
   * @param dest The destination position to find a matching move for.
   * @return An optional containing the matching move, or std::nullopt if none
   * found.
   */
  std::optional<BraendiDog::Move> findMatchingMove(
      const BraendiDog::Position& dest) const;

  /**
   * @brief Calculate the number of steps between two positions
   * @param from Starting position
   * @param to Destination position
   * @return Number of steps moved
   */
  size_t calculateMoveSteps(const BraendiDog::Position& from,
                            const BraendiDog::Position& to) const;

  /**
   * @brief Fold the current turn, sending a skip turn request to the server.
   */
  void foldTurn();

  // callbacks UI can attach to for visual feedback
  /**
   * @brief Callback function to update the status message in the UI.
   * @param s The status message to display.
   */
  std::function<void(const std::string&)> statusCallback;
  /**
   * @brief Callback function to notify when the selection has changed.
   */
  std::function<void()> selectionChangedCallback;

  /**
   * @brief Clear the current selections (card and marble).
   */
  void clearSelection();

 private:
  Client* client_;        ///< Pointer to the client for sending messages
  size_t myPlayerIndex_;  ///< Index of the local player
  BraendiDog::GameState& gameState_;  ///< Reference to the current game state
  // Legal Options
  std::vector<BraendiDog::Move> legalMoves_;     ///< Cached legal moves for the
                                                 ///< current turn
  std::vector<BraendiDog::Move> filteredMoves_;  ///< Filtered moves based on
                                                 ///< selection
  std::vector<BraendiDog::Move>
      jokerMoves_;  ///< Joker moves based on selected rank
  std::vector<BraendiDog::Move>
      sevenMoves_;  ///< Seven moves based on selected splits
  std::optional<BraendiDog::GameState>
      sevenTempGameState_;  ///< Temporary game state during Seven move building
  BraendiDog::Move builtSevenMove_;  ///< Currently built Seven move
  size_t
      totalSevenMoveValue_;  ///< Total value of the currently built Seven move

  // Selection state
  int selectedHandIndex_;  ///< Index of the selected card in hand, -1 if none
  int selectedCardID_;     ///< ID of the selected card
  std::optional<BraendiDog::MarbleIdentifier>
      selectedMarble_;     ///< Selected marble
  int jokerSelectedRank_;  ///< The rank selected for a Joker card, -1 if not
                           ///< set
};