/**
 * @file game.hpp
 * @brief Core game logic for Brändi Dog and GameState management.
 */

#pragma once

#include <array>
#include <cstddef>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "shared/game_objects.hpp"
#include "shared/game_types.hpp"

/**
 * @namespace BraendiDog
 * @brief Namespace for BraendiDog game objects and logic.
 *
 * Used throughout the full BraendiDog implementation. Encapsulates all game
 * related classes, functions, and types.
 */
namespace BraendiDog {

/**
 * @brief GameState implementation holding the full state of a BraendiDog game.
 *
 * Clients and Server will own their own GameState instances.
 * The GameState encapsulates all game objects and provides methods to
 * manipulate and query the game state.
 */
class GameState {
 private:
  std::array<Card, 54> deck;  ///< Full deck of cards (52 typical BraendiDog
                              ///< cards plus 2 jokers).
  std::array<std::optional<Player>, 4>
      players;  ///< Array of all player slots holding optional present player
                ///< instances.
  size_t currentPlayer;  ///< Index of the current player (who's turn it is).
  size_t
      roundStartPlayer;  ///< Index of the player who started the current round.
  size_t roundCardCount;  ///< Number of cards dealt in the current round.
  std::optional<size_t>
      lastPlayedCard;  ///< ID of the last played card for display.
  std::array<std::optional<int>, 4>
      leaderBoard;  ///< Player IDs in finishing order.

 public:
  // Constructors
  /**
   * @brief Default constructor for GameState used during serialization.
   */
  GameState() = default;
  /**
   * @brief Constructor for GameState initializing players.
   * @param gamePlayers Array of optional player names for each of the 4 player
   * slots.
   * @note Present players need a name, absent players are represented by
   * std::nullopt.
   */
  GameState(const std::array<std::optional<std::string>, 4>&
                gamePlayers);  // takes player names in array representing 4
                               // players and IDs as indices

  // Friend declaration for JSON serialization
  /**
   * @brief Friend declarations for JSON serialization and deserialization.
   * @param j Reference to a JSON object.
   * @param gs Constant reference to a GameState instance.
   */
  friend void to_json(nlohmann::json& j, const GameState& gs);
  /**
   * @brief Friend declaration for JSON deserialization.
   * @param j Constant reference to a JSON object.
   * @param gs Reference to a GameState instance.
   */
  friend void from_json(const nlohmann::json& j, GameState& gs);

  // Getters
  /**
   * @brief Get the full deck of cards.
   * @return Constant reference to the array of cards representing the deck.
   */
  const std::array<Card, 54>& getDeck() const;
  /**
   * @brief Get the array of players.
   * @return Reference to the array of optional Player instances.
   * @note const and non-const versions provided.
   */
  const std::array<std::optional<Player>, 4>& getPlayers() const;
  std::array<std::optional<BraendiDog::Player>, 4>& getPlayers();

  /**
   * @brief Get a player by index.
   * @param index Index of the player to retrieve.
   */
  const std::optional<BraendiDog::Player>& getPlayerByIndex(size_t index) const;
  std::optional<BraendiDog::Player>& getPlayerByIndex(size_t index);

  /**
   * @brief Get the index of the current player.
   * @return Index of the current player.
   */
  size_t getCurrentPlayer() const;
  /**
   * @brief Get the index of the round start player.
   * @return Index of the player who started the current round.
   */
  size_t getRoundStartPlayer() const;
  /**
   * @brief Get the number of cards dealt in the current round.
   * @return Number of cards dealt in the current round.
   */
  size_t getRoundCardCount() const;
  /**
   * @brief Get the ID of the last played card.
   * @return Optional ID of the last played card.
   * @note Nullopt at round start.
   */
  std::optional<size_t> getLastPlayedCard() const;
  /**
   * @brief Get the leaderboard of finished players.
   * @return Constant reference to the array of player IDs in finishing order.
   */
  const std::array<std::optional<int>, 4>& getLeaderBoard() const;

  // Setters
  /**
   * @brief Set the current player index.
   * @param playerIndex Index of the player to set as current.
   */
  void setCurrentPlayer(size_t playerIndex);
  /**
   * @brief Update the current player to the next active (inRound) player.
   */
  void updateCurrentPlayer();
  /**
   * @brief Update the round start player to the next active (inGame) player.
   */
  void updateRoundStartPlayer();
  /**
   * @brief Update the round card count according to game rules : 6, 5, 4, 3, 2,
   * repeat
   */
  void updateRoundCardCount();
  /**
   * @brief Set the last played card ID.
   * @param cardID Optional ID of the last played card. Nullopt at round start.
   */
  void setLastPlayedCard(std::optional<size_t> cardID);
  /**
   * @brief Add player to leaderboard as finished.
   * @param playerID ID of the player to add.
   */
  void addLeaderBoardFinished(size_t playerID);
  /**
   * @brief Add player to leaderboard as unfinished.
   * @param playerID ID of the player to add.
   */
  void addLeaderBoardUnfinished(size_t playerID);
  /**
   * @brief Add player to leaderboard as disconnected.
   * @param playerID ID of the player to add.
   * TODO: Possibly handle disconect after finish differently.
   */
  void addLeaderBoardDisconnected(size_t playerID);

  // Methods
  /**
   * @brief Check if it's the turn of the specified player. (Called by clients
   * after game state update)
   * @param playerIndex Index of the player to check.
   * @return True if it's the player's turn, false otherwise.
   */
  bool isMyTurn(size_t playerIndex) const;

  /**
   * @brief Get count of active players in round.
   */
  size_t getActiveInRoundCount() const;

  /**
   * @brief Get count of active players in game.
   */
  size_t getActiveInGameCount() const;

  /**
   * @brief Check if the current round has ended.
   * @return True if the round has ended, false otherwise.
   */
  bool checkRoundEnd() const;

  /**
   * @brief Check if the game has ended.
   * @return True if the game has ended, false otherwise.
   */
  bool checkGameEnd() const;

  /**
   * @brief Get indices of active players in game.
   * @return A vector of indices of active players.
   */
  std::vector<size_t> getActivePlayerIndices() const;

  /**
   * @brief Deal cards to players.
   * @return A map of active player IDs to a vector of their dealt card IDs.
   */
  std::map<size_t, std::vector<size_t>> dealCards() const;

  /// Move Validation and Computation ///

  /**
   * @brief Checks if specified field is occupied by any marble.
   * @param pos Position to check.
   * @return Optional MarbleIdentifier of occupying marble, nullopt if
   * unoccupied.
   */
  std::optional<BraendiDog::MarbleIdentifier> isFieldOccupied(
      const Position& pos) const;

  /**
   * @brief Check Start Move validity and end position.
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  checkStartMove(const Position& marblePos) const;

  /**
   * @brief Check Simple Move validity and end position.
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  checkSimpleMove(const Position& marblePos, int moveValue) const;

  /**
   * @brief Check Seven Move validity and end position(s).
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  checkSevenMove(const Position& marblePos, int moveValue) const;

  /**
   * @brief Check Swap Move validity and end position(s).
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  checkSwapMove(const Position& marblePos) const;

  /**
   * @brief Check Joker Move validity and end position(s).
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  checkJokerMove() const;

  /**
   * @brief Validate a proposed move for a player.
   * @param marblePos Current position of the marble to move.
   * @param moveType Type of move being proposed.
   * @param moveValue Value associated with the move (e.g., card value).
   * @return Optional end position(s) if the move is valid, nullopt otherwise.
   */
  std::optional<std::vector<std::pair<MarbleIdentifier, Position>>>
  validateMove(const Card& card, const Position& marblePos,
               const std::pair<MoveType, int>& moveRule,
               bool sevenCall = false) const;

  /**
   * @brief Compute all legal plays for the current player given their hand and
   * marble positions.
   */
  std::vector<BraendiDog::Move> computeLegalMoves(
      std::optional<std::array<size_t, 3>> Special = std::nullopt,
      bool sevenCall = false) const;

  /**
   * @brief Check if folding with a Joker in Hand is valid for the current
   * player.
   * @return True if folding is valid, false otherwise.
   */
  bool validJokerFold() const;

  /**
   * @brief Check if folding with a Seven in Hand is valid for the current
   * player.
   * @return True if folding is valid, false otherwise.
   */
  bool validSevenFold() const;

  /**
   * @brief Check if the current player has any special moves.
   */
  std::pair<bool, bool> hasSpecialMoves() const;

  /**
   * @brief Check if the current player has any legal moves.
   */
  bool hasLegalMoves() const;

  /**
   * @brief Apply a move to update marble positions (client-side preview)
   * @param move The move to apply
   * @note This does not validate the move, only updates positions
   */
  void applyTempSevenMove(const Move& move);

  /// Server Game State Manipulation Methods ///
  /**

   */
  bool isValidTurn(const BraendiDog::Move& move = Move()) const;

  /**
   * @brief Perform all Turn-Round-Game logic checks and GameState updates.
   * @return Pair of booleans indicating (gameEnded, roundEnded).
   */
  std::pair<bool, bool> endTurn();

  /**
   * @brief Update  all CurrentPlayer related GameState attributes according to
   * the given move.
   */
  void executeFold();

  /**
   * @brief Update all Move and CurrentPlayer related GameState attributes
   * according to the given move.
   * @param move Move to execute.
   * @return True if the move was the players finish move, false otherwise.
   */
  bool executeMove(BraendiDog::Move move);

  /**
   * @brief Update a players attributes if disconnected.
   * @param playerIndex Index of the player to disconnect.
   */
  void disconnectPlayer(size_t playerIndex);
};

// Inline Serialization of GameState to JSON.
/**
 * @brief Inline Serialization of GameState to JSON.
 * @param j Reference to a JSON object.
 * @param gs Constant reference to a GameState instance.
 */
inline void to_json(nlohmann::json& j, const GameState& gs) {
  j["deck"] = gs.getDeck();
  j["players"] = gs.getPlayers();
  j["currentPlayer"] = gs.getCurrentPlayer();
  j["roundStartPlayer"] = gs.getRoundStartPlayer();
  j["roundCardCount"] = gs.getRoundCardCount();
  j["lastPlayedCard"] = gs.getLastPlayedCard();
  j["leaderBoard"] = gs.getLeaderBoard();
};

// Inline Deserialization of GameState from JSON.
/**
 * @brief Inline Deserialization of GameState from JSON.
 * @param j Constant reference to a JSON object.
 * @param gs Reference to a GameState instance.
 */
inline void from_json(const nlohmann::json& j, GameState& gs) {
  gs.deck = j.at("deck").get<std::array<Card, 54>>();
  gs.players = j.at("players").get<std::array<std::optional<Player>, 4>>();
  gs.currentPlayer = j.at("currentPlayer").get<size_t>();
  gs.roundStartPlayer = j.at("roundStartPlayer").get<size_t>();
  gs.roundCardCount = j.at("roundCardCount").get<size_t>();
  gs.lastPlayedCard = j.at("lastPlayedCard").get<std::optional<size_t>>();
  gs.leaderBoard = j.at("leaderBoard").get<std::array<std::optional<int>, 4>>();
};

}  // namespace BraendiDog
