/**
 * @file game_objects.hpp
 * @brief Core game objects for Brändi Dog. ("Physical" game entities like Cards
 * and Players)
 */

#pragma once

#include <cstddef>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

#include "shared/game_types.hpp"

namespace BraendiDog {

/**
 * @brief Card objects.
 */
class Card {
 private:
  Rank rank;  ///< Rank of the card.
  Suit suit;  ///< Suit of the card.
  std::vector<std::pair<MoveType, int>>
      moveRules;  ///< Move rules associated with the card.

 public:
  // Constructors and Methods
  /**
   * @brief Default constructor for Card used during serialization.
   */
  Card() = default;
  /**
   * @brief Constructor for Card initializing rank and suit.
   * @param r Rank of the card.
   * @param s Suit of the card.
   * @note Move rules are set based on rank and suit.
   */
  Card(Rank r, Suit s);

  // Getters
  /**
   * @brief Get the rank of the card.
   * @return Rank of the card.
   */
  Rank getRank() const;
  /**
   * @brief Get the suit of the card.
   * @return Suit of the card.
   */
  Suit getSuit() const;
  /**
   * @brief Get the move rules associated with the card.
   * @return Vector of move type and value pairs.
   */
  const std::vector<std::pair<MoveType, int>>& getMoveRules() const;

  // Operator Overloads (Needed for testing)
  /**
   * @brief Equality operator overload for Card.
   * @param other Constant reference to another Card instance.
   * @return True if both cards have the same rank and suit, false otherwise.
   */
  bool operator==(const Card& other) const {
    return rank == other.rank && suit == other.suit;
  }
  /**
   * @brief Inequality operator overload for Card.
   * @param other Constant reference to another Card instance.
   * @return True if the cards differ in rank or suit, false otherwise.
   */
  bool operator!=(const Card& other) const { return !(*this == other); }

  // Friend declaration for JSON serialization
  /**
   * @brief Friend declarations for JSON serialization and deserialization.
   * @param j Reference to a JSON object.
   * @param card Constant reference to a Card instance.
   */
  friend void to_json(nlohmann::json& j, const Card& card);
  /**
   * @brief Friend declaration for JSON deserialization.
   * @param j Constant reference to a JSON object.
   * @param card Reference to a Card instance.
   */
  friend void from_json(const nlohmann::json& j, Card& card);
};

/**
 * @brief Inline Serialization of Card to JSON.
 * @param j Reference to a JSON object.
 * @param card Constant reference to a Card instance.
 */
inline void to_json(nlohmann::json& j, const Card& card) {
  j["rank"] = static_cast<int>(card.getRank());
  j["suit"] = static_cast<int>(card.getSuit());
  j["moveRules"] = nlohmann::json::array();
  for (const auto& rule : card.getMoveRules()) {
    nlohmann::json ruleJson;
    ruleJson["type"] = static_cast<int>(rule.first);
    ruleJson["value"] = rule.second;
    j["moveRules"].push_back(ruleJson);
  }
};

/**
 * @brief Inline Deserialization of Card from JSON.
 * @param j Constant reference to a JSON object.
 * @param card Reference to a Card instance.
 */
inline void from_json(const nlohmann::json& j, Card& card) {
  card.rank = static_cast<Rank>(j.at("rank").get<int>());
  card.suit = static_cast<Suit>(j.at("suit").get<int>());
  card.moveRules.clear();
  for (const auto& ruleJson : j.at("moveRules")) {
    card.moveRules.emplace_back(
        static_cast<MoveType>(ruleJson.at("type").get<int>()),
        ruleJson.at("value").get<int>());
  };
};

/**
 * @brief Player object.
 */
class Player {
 private:
  // Constant attributes - no longer const due to deserialization implementation
  size_t id;          ///< player ID
  std::string name;   ///< chosen username
  size_t startField;  ///< starting field on the board -> 16*playerID

  // Status attributes
  std::optional<size_t> startBlocked;  ///< is starting field blocked
  bool activeInRound;  ///< is player active in the current round
  bool activeInGame;   ///< is player active in the current game

  // Game attributes
  std::array<Position, 4> marbles;  ///< player's marbles
  std::vector<size_t> hand;         ///< cards currently in player's hand

 public:
  // Constructors and Methods
  /**
   * @brief Default constructor for Player used during serialization.
   */
  Player() = default;
  /**
   * @brief Constructor for Player initializing ID and name.
   * @param pID Player ID.
   * @param pName Player name.
   */
  Player(size_t pID, const std::string& pName);

  // Getters
  /**
   * @brief Get the player ID.
   * @return Player ID.
   */
  size_t getId() const;
  /**
   * @brief Get the player name.
   * @return Player name.
   */
  std::string getName() const;
  /**
   * @brief Get the starting field of the player.
   * @return Starting field index.
   */
  size_t getStartField() const;
  /**
   * @brief Get the start blocked status of the player.
   * @return Optional index of blocked marble at start, nullopt if not blocked.
   */
  std::optional<size_t> getStartBlocked() const;
  /**
   * @brief Check if the starting field is blocked.
   * @return True if the starting field is blocked, false otherwise.
   */
  bool isStartBlocked() const;
  /**
   * @brief Check if the player is active in the current round.
   * @return True if active in round, false otherwise.
   */
  bool isActiveInRound() const;
  /**
   * @brief Check if the player is active in the game.
   * @return True if active in game, false otherwise.
   */
  bool isActiveInGame() const;
  /**
   * @brief Get the positions of the player's marbles.
   * @return Constant reference to an array of marble positions.
   */
  const std::array<Position, 4>& getMarbles() const;
  /**
   * @brief Get the position of a specific marble by index.
   * @param marbleIndex Index of the marble (0-3).
   * @return Constant reference to the position of the specified marble.
   */
  const Position& getMarblePosition(size_t marbleIndex) const;
  /**
   * @brief Get the marble index at a specific position.
   * @param pos Position to check.
   * @return Optional index of the marble at the given position, nullopt if
   * none.
   */
  std::optional<size_t> getMarbleIndexByPos(const Position& pos) const;
  /**
   * @brief Get the player's hand of card IDs.
   * @return Constant reference to a vector of card IDs in the player's hand.
   */
  const std::vector<size_t>& getHand() const;

  /**
   * @brief Check if the player's hand is empty.
   * @return True if the hand is empty, false otherwise.
   */
  bool isHandEmpty() const;

  /**
   * @brief Check if the player has a Joker card in hand.
   * @return True if a Joker card is present, false otherwise.
   */
  bool hasJokerInHand() const;

  /**
   * @brief Check if the player has a Seven card in hand.
   * @param rank Rank of the card to check for. (1-13)
   * @return True if a Seven card is present, false otherwise.
   */
  bool hasCardInHand(size_t rank) const;

  // Setters
  /**
   * @brief Set the position of a specific marble by index.
   * @param marbleIndex Index of the marble (0-3).
   * @param newPosition New position to set for the marble.
   */
  void setMarblePosition(size_t marbleIndex, const Position& newPosition);
  /**
   * @brief Set the player's hand of card IDs.
   * @param cardIds Vector of card IDs to set as the player's hand.
   */
  void setHand(const std::vector<size_t>& cardIds);
  /**
   * @brief Set the start blocked status of the player.
   * @param blocked Optional index of blocked marble at start.
   * @note Use nullopt to indicate not blocked.
   */
  void setStartBlocked(size_t blocked);
  /**
   * @brief Reset the start blocked status of the player.
   */
  void resetStartBlocked();
  /**
   * @brief Set the active in game status of the player.
   * @param isActive True to set active, false to set inactive.
   */
  void setActiveInRound(bool isActive);

  /**
   * @brief Set the active in game status of the player.
   * @param isActive True to set active, false to set inactive.
   */
  void setActiveInGame(bool isActive);

  // Methods
  /**
   * @brief Remove and return a card ID from the player's hand by index.
   * @param handIndex Index of the card in the player's hand to remove.
   * @return The card ID that was removed from the hand.
   */
  size_t popCardFromHand(size_t handIndex);
  /**
   * @brief Check if the player has finished the game.
   * @return True if all marbles are in the home position, false otherwise.
   */
  bool checkFinished() const;

  // Friend declaration for JSON serialization
  /**
   * @brief Friend declarations for JSON serialization and deserialization.
   * @param j Reference to a JSON object.
   * @param player Constant reference to a Player instance.
   */
  friend void to_json(nlohmann::json& j, const Player& player);
  /**
   * @brief Friend declaration for JSON deserialization.
   * @param j Constant reference to a JSON object.
   * @param player Reference to a Player instance.
   */
  friend void from_json(const nlohmann::json& j, Player& player);
};

/**
 * @brief Inline Serialization of Player to JSON.
 * @param j Reference to a JSON object.
 * @param player Constant reference to a Player instance.
 */
inline void to_json(nlohmann::json& j, const Player& player) {
  j["id"] = player.getId();
  j["name"] = player.getName();
  j["startField"] = player.getStartField();
  j["startBlocked"] = player.getStartBlocked().has_value()
                          ? nlohmann::json(player.getStartBlocked().value())
                          : nlohmann::json(nullptr);
  j["marbles"] = player.getMarbles();
  j["activeInRound"] = player.isActiveInRound();
  j["activeInGame"] = player.isActiveInGame();
};

/**
 * @brief Inline Deserialization of Player from JSON.
 * @param j Constant reference to a JSON object.
 * @param player Reference to a Player instance.
 */
inline void from_json(const nlohmann::json& j, Player& player) {
  player.id = j.at("id").get<size_t>();
  player.name = j.at("name").get<std::string>();
  player.startField = j.at("startField").get<size_t>();
  if (j.at("startBlocked").is_null()) {
    player.startBlocked = std::nullopt;
  } else {
    player.startBlocked = j.at("startBlocked").get<size_t>();
  }
  player.marbles = j.at("marbles").get<std::array<Position, 4>>();
  player.activeInRound = j.at("activeInRound").get<bool>();
  player.activeInGame = j.at("activeInGame").get<bool>();
};

}  // namespace BraendiDog