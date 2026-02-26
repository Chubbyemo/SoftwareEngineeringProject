/**
 * @file game_types.hpp
 * @brief Core game types and enums for Brändi Dog. (Descriptive structs and
 * enums for abstract game concepts)
 */

#pragma once

#include <cstddef>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include <vector>

namespace BraendiDog {

/**
 * @brief Card ranks for Brändi Dog.
 */
enum class Rank {
  ACE,    ///< Ace rank
  TWO,    ///< 2 rank
  THREE,  ///< 3 rank
  FOUR,   ///< 4 rank
  FIVE,   ///< 5 rank
  SIX,    ///< 6 rank
  SEVEN,  ///< 7 rank
  EIGHT,  ///< 8 rank
  NINE,   ///< 9 rank
  TEN,    ///< 10 rank
  JACK,   ///< Jack rank
  QUEEN,  ///< Queen rank
  KING,   ///< King rank
  JOKER   ///< Joker rank
};

/**
 * @brief Card suits for Brändi Dog.
 */
enum class Suit {
  CLUBS,     ///< Clubs suit
  DIAMONDS,  ///< Diamonds suit
  HEARTS,    ///< Hearts suit
  SPADES,    ///< Spades suit
  JOKER      ///< Joker suit (trivial)
};

/**
 * @brief Possible board locations for a marble.
 *
 * The three board areas impact move logic.
 */
enum class BoardLocation {
  HOME,   ///< Home area
  TRACK,  ///< Track area
  FINISH  ///< Finish area
};

/**
 * @brief Different types of moves in Brändi Dog.
 */
enum class MoveType {
  SIMPLE,  ///< Simple walking move (paired with a value)
  SEVEN,   ///< Indicates a split move of 7.
  SWAP,    ///< Swap positions with another player's marble.
  JOKER,   ///< Triggers rank selection for joker card.
  START    ///< Allows moving a marble from home to starting position on board.
};

/**
 * @brief Represents the position on the Brändi Dog board.
 */
struct Position {
  BoardLocation boardLocation;  ///< Location on the board (HOME, TRACK, FINISH)
  size_t index;                 ///< Index on the track, finish area or home.
  size_t playerID;  ///< Owner player index for marbles in HOME or FINISH areas

  // Constructors and Methods
  /**
   * @brief Default constructor for Position used during serialization.
   */
  Position() = default;
  /**
   * @brief Constructor for Position initializing all attributes.
   * @param loc Board location type.
   * @param idx Index on the board.
   * @param pID Player ID owning the marble.
   */
  Position(BoardLocation loc, size_t idx, size_t pID);

  // Methods
  /**
   * @brief Check equality with another Position.
   * @param other The other Position to compare with.
   */
  bool equals(const Position& other) const;

  /**
   * @brief Equality operator overload for Position.
   * @param other Constant reference to another Position instance.
   */
  bool operator==(const Position& other) const {
    return equals(other);  // Reuse your existing equals method
  }
  /**
   * @brief Inequality operator overload for Position.
   * @param other Constant reference to another Position instance.
   */
  bool operator!=(const Position& other) const { return !equals(other); }

  /**
   * @brief Check if the position is in the home area.
   * @return True if in home area, false otherwise.
   */
  bool isInHome() const;
  /**
   * @brief Check if the position is in the finish area.
   * @return True if in finish area, false otherwise.
   */
  bool isInFinish() const;
};

/**
 * @brief Inline Serialization of Position to JSON.
 * @param j Reference to a JSON object.
 * @param pos Constant reference to a Position instance.
 */
inline void to_json(nlohmann::json& j, const Position& pos) {
  j["boardLocation"] = static_cast<int>(pos.boardLocation);
  j["index"] = pos.index;
  j["playerID"] = pos.playerID;
};

/**
 * @brief Inline Deserialization of Position from JSON.
 * @param j Constant reference to a JSON object.
 * @param pos Reference to a Position instance.
 */
inline void from_json(const nlohmann::json& j, Position& pos) {
  pos.boardLocation =
      static_cast<BoardLocation>(j.at("boardLocation").get<int>());
  pos.index = j.at("index").get<size_t>();
  pos.playerID = j.at("playerID").get<size_t>();
};

/**
 * @brief Represents a marble identifier for moves.
 * @note Easily serializable and lookup of O(1).
 */
struct MarbleIdentifier {
  size_t playerID;   ///< ID of the player owning the marble (index in players
                     ///< array of GameState)
  size_t marbleIdx;  ///< Index of the marble in the player's marbles array

  // Constructors and Methods
  /**
   * @brief Default constructor for MarbleIdentifier used during serialization.
   */
  MarbleIdentifier() = default;
  /**
   * @brief Constructor for MarbleIdentifier initializing all attributes.
   * @param pID Player ID owning the marble.
   * @param mIdx Index of the marble.
   */
  MarbleIdentifier(size_t pID, size_t mIdx);
};

/**
 * @brief Inline Serialization of MarbleIdentifier to JSON.
 * @param j Reference to a JSON object.
 * @param mid Constant reference to a MarbleIdentifier instance.
 */
inline void to_json(nlohmann::json& j, const MarbleIdentifier& mid) {
  j["playerID"] = mid.playerID;
  j["marbleIdx"] = mid.marbleIdx;
};

/**
 * @brief Inline Deserialization of MarbleIdentifier from JSON.
 * @param j Constant reference to a JSON object.
 * @param mid Reference to a MarbleIdentifier instance.
 */
inline void from_json(const nlohmann::json& j, MarbleIdentifier& mid) {
  mid.playerID = j.at("playerID").get<size_t>();
  mid.marbleIdx = j.at("marbleIdx").get<size_t>();
};

/**
 * @brief Represents one game move and the card utilised for it.
 *
 * Includes where said card lies in the player's hand, and what marbles get
 * moved. The move object includes actively moved marbles and all "passively"
 * affected marbles in case of swaps or sending home.
 */
struct Move {
  size_t cardID;  ///< The card used for the move, referenced by it's index in
                  ///< the deck
  size_t handIndex;  ///< The index of the card in the player's hand
  std::vector<std::pair<MarbleIdentifier, Position>>
      movements;  ///< The marbles being moved by this move and their target
                  ///< positions

  // Constructors and Methods
  /**
   * @brief Default constructor for Move used during serialization.
   */
  Move() = default;
  /**
   * @brief Constructor for Move initializing all attributes.
   * @param cID Card ID used for the move.
   * @param hIndex Index of the card in the player's hand.
   * @param moves Vector of marble movements.
   */
  Move(size_t cID, size_t hIndex,
       std::vector<std::pair<MarbleIdentifier, Position>> moves);

  // Getters
  /**
   * @brief Get the Card ID used for the move.
   * @return Card ID.
   */
  size_t getCardID() const;
  /**
   * @brief Get the hand index of the card used for the move.
   * @return Hand index.
   */
  size_t getHandIndex() const;
  /**
   * @brief Get the marble movements associated with the move.
   * @return Vector of marble movements.
   */
  const std::vector<std::pair<MarbleIdentifier, Position>>& getMovements()
      const;
};

/**
 * @brief Inline Serialization of Move to JSON.
 * @param j Reference to a JSON object.
 * @param move Constant reference to a Move instance.
 */
inline void to_json(nlohmann::json& j, const Move& move) {
  j["cardID"] = move.cardID;
  j["handIndex"] = move.handIndex;
  j["movements"] = move.movements;
};

/**
 * @brief Inline Deserialization of Move from JSON.
 * @param j Constant reference to a JSON object.
 * @param move Reference to a Move instance.
 */
inline void from_json(const nlohmann::json& j, Move& move) {
  move.cardID = j.at("cardID").get<size_t>();
  move.handIndex = j.at("handIndex").get<size_t>();
  move.movements =
      j.at("movements")
          .get<std::vector<std::pair<MarbleIdentifier, Position>>>();
};

}  // namespace BraendiDog

// Extend nlohmann::json for std::optional<T>
namespace nlohmann {
// Generic serialization for std::optional<T>
/**
 * @brief Generic serialization for std::optional<T>.
 * @tparam T The type contained within the std::optional.
 */
template <typename T>
struct adl_serializer<std::optional<T>> {
  static void to_json(json& j, const std::optional<T>& opt) {
    if (opt.has_value()) {
      j = *opt;  // Serialize the contained value
    } else {
      j = nullptr;  // Serialize as null
    }
  }

  static void from_json(const json& j, std::optional<T>& opt) {
    if (j.is_null()) {
      opt = std::nullopt;
    } else {
      opt = j.get<T>();  // Deserialize to the contained type
    }
  }
};

}  // namespace nlohmann