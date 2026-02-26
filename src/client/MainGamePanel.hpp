#pragma once

#include <wx/wx.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "client/MovePhaseController.hpp"
#include "shared/game.hpp"

// Forward declarations
class Client;

class MainGameFrame : public wxFrame {
 public:
  /**
   * @brief Constructor for MainGameFrame
   * @param title The window title
   * @param client Pointer to the client instance
   * @param numPlayers Number of players in the game
   */
  MainGameFrame(const wxString& title, Client* client, unsigned int numPlayers);

 private:
  // ============================================================================
  // Game State & Controllers
  // ============================================================================

  BraendiDog::GameState gameState_;  ///< Local copy of the current game state
  std::unique_ptr<MovePhaseController>
      moveController;       ///< Controller for managing move phases
  Client* client;           ///< Reference to the client instance
  unsigned int numPlayers;  ///< Number of players in the game

  // ============================================================================
  // Scaling & Dimensions
  // ============================================================================

  double currentScaleFactor_;      ///< Current board scale factor
  int scaledSpotRadius_;           ///< Scaled board spot circle radius
  int scaledMarbleRadius_;         ///< Scaled marble circle radius
  int scaledIconSize_;             ///< Scaled user icon size
  double iconDistanceMultiplier_;  ///< Distance multiplier for icon placement
                                   ///< from board center
  const int MIN_WINDOW_WIDTH;      ///< Minimum board width for scaling
  const int MIN_WINDOW_HEIGHT;     ///< Minimum board height for scaling
  int boardMinX;                   ///< Minimum X coordinate of board bounds
  int boardMinY;                   ///< Minimum Y coordinate of board bounds
  int boardMaxX;                   ///< Maximum X coordinate of board bounds
  int boardMaxY;                   ///< Maximum Y coordinate of board bounds
  int boardWidth;                  ///< Width of the board
  int boardHeight;                 ///< Height of the board
  double boardScaleFactor;  ///< Scale factor applied to board coordinates

  // ============================================================================
  // Board Data
  // ============================================================================

  std::vector<wxPoint> boardSpots;  ///< Positions of all spots on the board
  std::map<std::string, std::vector<int>>
      spotGroups;  ///< Grouped board spots by type (home, finish, track, etc.)

  // ============================================================================
  // UI Components
  // ============================================================================

  wxPanel* panel;                 ///< Main panel containing all UI elements
  wxStaticText* statusText;       ///< Status text display
  wxStaticText* playerInfoText;   ///< Player information text display
  wxStaticText* placeholderText;  ///< Placeholder text shown before game starts
  wxStaticBitmap* rulesButton;    ///< Clickable image to display game rules
  wxStaticBitmap* diceIcon;       ///< Dice icon display

  // ============================================================================
  // Player Data
  // ============================================================================

  wxStaticText* playerNameLabels[4];  ///< Player name labels for each player
  wxPoint userIconPositions[4];  ///< Positions of user icons for each player
  std::vector<wxStaticBitmap*>
      playerHandCards;  ///< Visual representations of cards in player's hand
  int highlightedCardIndex =
      -1;  ///< Index of currently highlighted card in hand
  std::vector<int> disconnectedPlayers_;

  // ============================================================================
  // Resource Management
  // ============================================================================

  wxBitmap yellowUserBitmap;  ///< Yellow player icon bitmap
  wxBitmap redUserBitmap;     ///< Red player icon bitmap
  wxBitmap blueUserBitmap;    ///< Blue player icon bitmap
  wxBitmap greenUserBitmap;   ///< Green player icon bitmap
  wxBitmap greyUserBitmap;    ///< Grey player icon bitmap
  wxBitmap rulesIconBitmap;   ///< Rules icon bitmap
  wxBitmap rulesImageBitmap;  ///< Full rules image bitmap
  wxBitmap diceImages[6];     ///< Dice face images
  wxBitmap rankBitmaps[3];    ///< Rank icon bitmaps

  // ============================================================================
  // Initialization Methods
  // ============================================================================

  /**
   * @brief Load player icon bitmaps from files
   * @return true if all icons loaded successfully, false otherwise
   */
  bool LoadPlayerIcons();

  /**
   * @brief Create and initialize all UI components
   */
  void CreateUIComponents();

  /**
   * @brief Initialize board data including spot positions and groups
   */
  void InitializeBoardData();

  // ============================================================================
  // Calculation Methods
  // ============================================================================

  /**
   * @brief Calculate user icon positions based on current board size and layout
   */
  void CalculateUserIconPositions();

  /**
   * @brief Place an icon along a line from center to target point
   * @param playerIndex Index of the player
   * @param slot Slot position (0-3 representing corners)
   * @param centerX X coordinate of board center
   * @param centerY Y coordinate of board center
   * @param targetPoint Target point for icon placement
   * @param iconSize Size of the icon
   */
  void PlaceIconAlongLine(int playerIndex, int slot, int centerX, int centerY,
                          const wxPoint& targetPoint, int iconSize);

  /**
   * @brief Update positions of player name labels
   * @param iconSize Size of the player icons
   */
  void UpdatePlayerNameLabelsPositions(int iconSize);

  /**
   * @brief Get the bounding rectangle of the board
   * @return Rectangle containing all board spots
   */
  wxRect GetBoardRect() const;

  /**
   * @brief Recenter the board on the panel after resize
   */
  void RecenterBoard();

  /**
   * @brief Calculate the center point of a group of board spots
   * @param indices Vector of spot indices
   * @return Center point of the group
   */
  wxPoint getGroupCenter(const std::vector<int>& indices) const;

  /**
   * @brief Determine which slot (corner) a position belongs to
   * @param center Center point of the board
   * @param p Point to check
   * @return Slot index (0-3)
   */
  int determineSlotFromPosition(const wxPoint& center, const wxPoint& p) const;

  /**
   * @brief Get the rotated slot position for a player after board rotation
   * @param playerIndex Index of the player
   * @return Slot index after rotation
   */
  int getRotatedSlotForPlayer(int playerIndex) const;

  /**
   * @brief Convert spot index to game position
   * @param spotIndex Index of the spot on the board
   * @return BraendiDog position object
   */
  BraendiDog::Position spotIndexToPosition(int spotIndex) const;

  /**
   * @brief Get display name for a player
   * @param i Player index
   * @return Player's display name
   */
  std::string getPlayerDisplayName(size_t i) const;

  // ============================================================================
  // Drawing Methods
  // ============================================================================
  wxPoint getPositionCenter(const BraendiDog::Position& pos) const;

  /**
   * @brief Draw highlight around the selected card in player's hand
   * @param dc Device context for drawing
   */
  void DrawCardHighlight(wxDC& dc);

  /**
   * @brief Draw the last played card on the board
   * @param dc Device context for drawing
   */
  void DrawLastPlayedCard(wxDC& dc);

  /**
   * @brief Handle paint events to redraw the board and marbles
   * @param event Paint event
   */
  void OnPaint(wxPaintEvent& event);

  /**
   * @brief Draw the board including spots and player icons
   * @param dc Device context for drawing
   */
  void DrawBoard(wxDC& dc);

  /**
   * @brief Draw all player marbles on the board
   * @param dc Device context for drawing
   */
  void DrawMarbles(wxDC& dc);

  /**
   * @brief Handle server update events
   * @param event Thread event containing server message
   */
  void OnServerUpdate(wxThreadEvent& event);

  /**
   * @brief Handle window resize events
   * @param event Size event
   */
  void OnResize(wxSizeEvent& event);

  /**
   * @brief Handle marble click events on the board
   * @param event Mouse event
   */
  void OnMarbleClicked(wxMouseEvent& event);

  /**
   * @brief Handle hand card click events
   * @param cardIndex Index of the clicked card
   */
  void OnHandCardClicked(size_t cardIndex);

  /**
   * @brief Shows a popup dialog for Joker card rank selection
   * @param handIndex Index of the Joker card in player's hand
   * @return True if a rank was selected, false if canceled
   */
  bool ShowJokerSelectionPopup(int handIndex);

  // ============================================================================
  // Game Logic & Update Methods
  // ============================================================================

  /**
   * @brief Take a turn if it's the current player's turn
   */
  void takeTurn() const;

  /**
   * @brief Update the game state and refresh UI
   */
  void UpdateGamestate();

  /**
   * @brief Update positions of UI elements
   */
  void UpdatePositions();

  /**
   * @brief Update the visual display of player's hand cards
   */
  void UpdatePlayerHand();

  /**
   * @brief Show rules dialog when rules button is clicked
   * @param event Button event
   */
  void OnRulesButtonClicked(wxCommandEvent& event);

  /**
   * @brief Update the dice icon based on current card count
   * @param cardCount Number of cards dealt to each player
   */
  void UpdateDiceIcon(int cardCount);

  /**
   * @brief Checks if a card at the specified index is a Joker
   * @param handIndex Index in player's hand
   * @return True if the card is a Joker
   */
  bool IsJokerCard(int handIndex) const;

  /**
   * @brief Checks if a card at the specified index is a Seven
   * @param handIndex Index in player's hand
   * @return True if the card is a Seven
   */
  bool IsSevenCard(int handIndex) const;

  // ============================================================================
  // Debug & Utility Methods
  // ============================================================================

  /**
   * @brief Debug output for user icon positions
   */
  void testing_DebugUserIconPositions();

  /**
   * @brief Get the image index for a card
   * @param cardIndex Index of the card in the deck
   * @return Image index for loading the card image
   */
  size_t getCardImageIndex(size_t cardIndex) const {
    return cardIndex + 1;  // image starts from Poker_1
  }

  // ============================================================================
  // End of game
  // ============================================================================

  /**
   * @brief Show results
   * @param leaderboard
   */
  void ShowResults(const std::array<std::optional<int>, 4>& leaderboard);
};