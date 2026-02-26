#include "MainGamePanel.hpp"

#include <wx/event.h>
#include <wx/statline.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <optional>

#include "client/client.hpp"
#include "shared/messages.hpp"

// MainGameFrame
MainGameFrame::MainGameFrame(const wxString& title, Client* client,
                             const unsigned int numPlayers)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 750)),
      client(client),
      numPlayers(numPlayers),
      gameState_(BraendiDog::GameState()),
      currentScaleFactor_(1.0),
      scaledSpotRadius_(12),
      scaledMarbleRadius_(10),
      scaledIconSize_(80),
      iconDistanceMultiplier_(1.35),
      playerNameLabels{nullptr, nullptr, nullptr, nullptr},
      MIN_WINDOW_HEIGHT(750),
      MIN_WINDOW_WIDTH(900)
// Incredibly important for 2 Player games
{
  std::cout << "Starting MainGameFrame initialization..." << std::endl;

  try {
    // 1. Create the main panel first (most critical UI element)
    std::cout << "Creating main panel..." << std::endl;
    panel = new wxPanel(this, wxID_ANY);
    if (!panel) {
      throw std::runtime_error("Failed to create main panel");
    }
    panel->SetBackgroundColour(*wxWHITE);
    SetMinSize(wxSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT));
    // Force proper panel size before initializing
    wxSize panelSize = panel->GetSize();
    std::cout << "Initial panel size: " << panelSize.GetWidth() << "x"
              << panelSize.GetHeight() << std::endl;
    // Panel is 20x20 (too small), force it to proper size
    panel->SetSize(wxSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT));
    // Re-fetch panel size
    panelSize = panel->GetSize();
    std::cout << "Panel size after SetSize: " << panelSize.GetWidth() << "x"
              << panelSize.GetHeight() << std::endl;

    // 2. Set up event handlers
    std::cout << "Setting up event handlers..." << std::endl;
    this->Bind(wxEVT_THREAD, &MainGameFrame::OnServerUpdate, this);
    this->Bind(wxEVT_SIZE, &MainGameFrame::OnResize, this);
    panel->Bind(wxEVT_PAINT, &MainGameFrame::OnPaint, this);
    panel->Bind(wxEVT_LEFT_DOWN, &MainGameFrame::OnMarbleClicked, this);

    // 3. Initialize game data
    std::cout << "Initializing game data..." << std::endl;
    InitializeBoardData();

    // 4. Load resources
    std::cout << "Loading resources..." << std::endl;
    if (!LoadPlayerIcons()) {
      throw std::runtime_error("Failed to load player icons");
    }

    // 5. Create UI components
    std::cout << "Creating UI components..." << std::endl;
    CreateUIComponents();

    // 6. Set up move controller
    std::cout << "Setting up move controller..." << std::endl;
    moveController = std::make_unique<MovePhaseController>(
        client, static_cast<size_t>(client->getPlayerIndex()), gameState_);
    moveController->statusCallback = [this](const std::string& s) {
      if (statusText) statusText->SetLabel(s);
    };
    moveController->selectionChangedCallback = [this]() {
      if (panel) panel->Refresh();
    };

    // 7. Final setup
    std::cout << "Performing final setup..." << std::endl;
    // Initialize with proper panel size
    std::cout << "Initializing board data..." << std::endl;
    CalculateUserIconPositions();

    std::cout << "Board initialization complete" << std::endl;
    // Force initial paint:
    panel->Refresh();

    // 8. Set up client callback last
    std::cout << "Setting up client callback..." << std::endl;
    client->setUpdateCallback([this](const std::string& message) {
      auto evt = new wxThreadEvent(wxEVT_THREAD, wxID_ANY);
      evt->SetString(message);
      wxQueueEvent(this, evt);
    });

    std::cout << "MainGameFrame initialization complete" << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error during MainGameFrame initialization: " << e.what()
              << std::endl;
    throw;  // Re-throw to ensure proper error handling
  }
}

void MainGameFrame::CreateUIComponents() {
  // Status text
  statusText = new wxStaticText(panel, wxID_ANY, "Waiting for game to start...",
                                wxPoint(10, 10));
  statusText->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                             wxFONTWEIGHT_NORMAL));

  // Player info text
  playerInfoText =
      new wxStaticText(panel, wxID_ANY, "Player Info", wxPoint(10, 40));

  // Placeholder text
  placeholderText = new wxStaticText(
      panel, wxID_ANY, "Brändi Dog - Game Screen", wxPoint(300, 250));
  placeholderText->SetFont(
      wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

  // Player name labels
  for (int i = 0; i < 4; i++) {
    // Skip empty player slots
    if (client->getPlayerList()[i].name.empty()) {
      continue;
    }
    std::string name = getPlayerDisplayName(i);
    wxString playerName = wxString::FromUTF8(name.c_str());

    playerNameLabels[i] =
        new wxStaticText(panel, wxID_ANY, playerName, wxDefaultPosition);
    playerNameLabels[i]->SetForegroundColour(wxColour(0, 0, 0));
    playerNameLabels[i]->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT,
                                        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  }

  // Rules button
  if (!rulesIconBitmap.IsOk()) {
    rulesIconBitmap = wxBitmap("../assets/rules_icon.png", wxBITMAP_TYPE_PNG);
  }

  if (rulesIconBitmap.IsOk()) {
    wxRect boardRect = GetBoardRect();
    rulesButton = new wxStaticBitmap(
        panel, wxID_ANY, rulesIconBitmap,
        wxPoint(boardRect.GetRight() + 90,
                boardRect.GetY() + boardRect.GetHeight() / 2 + 40));
    rulesButton->SetToolTip("Show Game Rules");

    // Bind click event to rules button
    rulesButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event) {
      wxCommandEvent cmdEvent(wxEVT_COMMAND_BUTTON_CLICKED);
      OnRulesButtonClicked(cmdEvent);
    });
  }

  // Create dice icon (independent of rules button)
  if (diceImages[5].IsOk()) {
    wxRect boardRect = GetBoardRect();
    diceIcon = new wxStaticBitmap(
        panel, wxID_ANY, diceImages[5],
        wxPoint(boardRect.GetRight() + 90,
                boardRect.GetY() + boardRect.GetHeight() / 2 - 30));
    diceIcon->SetToolTip("Current deal: 6 cards per player");
  }
}

bool MainGameFrame::LoadPlayerIcons() {
  try {
    // Load player icons
    yellowUserBitmap =
        wxBitmap("../assets/user_icons/yellow_user.png", wxBITMAP_TYPE_PNG);
    redUserBitmap =
        wxBitmap("../assets/user_icons/red_user.png", wxBITMAP_TYPE_PNG);
    blueUserBitmap =
        wxBitmap("../assets/user_icons/blue_user.png", wxBITMAP_TYPE_PNG);
    greenUserBitmap =
        wxBitmap("../assets/user_icons/green_user.png", wxBITMAP_TYPE_PNG);
    greyUserBitmap =
        wxBitmap("../assets/user_icons/grey_user.png", wxBITMAP_TYPE_PNG);
    wxImage rulesIconImage =
        wxImage("../assets/rules_icon.png", wxBITMAP_TYPE_PNG);
    // Load and rescale rules icon
    int buttonSize = 70;
    rulesIconImage.Rescale(buttonSize, buttonSize, wxIMAGE_QUALITY_HIGH);
    rulesIconBitmap = wxBitmap(rulesIconImage);
    rulesImageBitmap = wxBitmap("../assets/Rules.png", wxBITMAP_TYPE_PNG);
    // Load and rescale dice images
    int diceSize = 60;
    for (int i = 0; i < 6; i++) {
      wxString filename = wxString::Format("../assets/dice/dice_%d.png", i + 1);
      wxImage diceImage = wxImage(filename, wxBITMAP_TYPE_PNG);

      if (diceImage.IsOk()) {
        diceImage.Rescale(diceSize, diceSize, wxIMAGE_QUALITY_HIGH);
        diceImages[i] = wxBitmap(diceImage);
      } else {
        std::cerr << "Failed to load dice image: " << filename << std::endl;
      }
    }
    // Load rank icons
    for (int i = 0; i < 3; ++i) {
      wxString filename =
          wxString::Format("../assets/ranks/rank_%d.png", i + 1);
      wxBitmap bmp(filename, wxBITMAP_TYPE_PNG);
      if (!bmp.IsOk()) {
        std::cerr << "Failed to load rank icon: " << filename << std::endl;
      }
      rankBitmaps[i] = bmp;
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Failed to load player icons: " << e.what() << std::endl;
    return false;
  }
}

wxRect MainGameFrame::GetBoardRect() const {
  int minX = INT_MAX, minY = INT_MAX, maxX = 0, maxY = 0;
  for (const auto& point : boardSpots) {
    minX = std::min(minX, point.x);
    minY = std::min(minY, point.y);
    maxX = std::max(maxX, point.x);
    maxY = std::max(maxY, point.y);
  }
  return wxRect(minX, minY, maxX - minX, maxY - minY);
}

void MainGameFrame::InitializeBoardData() {
  // Initialize all points on the board
  std::vector<wxPoint> points = {
      // anti-clockwise
      wxPoint(179, 749), wxPoint(212, 716),
      wxPoint(245, 683), wxPoint(278, 650),  // 0,1,2,3
      wxPoint(311, 617), wxPoint(355, 631),
      wxPoint(403, 635), wxPoint(447, 631),  // 4,5,6,7
      wxPoint(490, 618), wxPoint(523, 651),
      wxPoint(556, 684), wxPoint(589, 717),  // 8,9,10,11
      wxPoint(622, 750), wxPoint(659, 724),
      wxPoint(693, 694), wxPoint(724, 660),  // 12,13,14,15
      wxPoint(750, 623), wxPoint(717, 590),
      wxPoint(684, 557), wxPoint(651, 524),  // 16,17,18,19
      wxPoint(618, 491), wxPoint(632, 446),
      wxPoint(636, 400), wxPoint(632, 355),  // 20,21,22,23
      wxPoint(618, 311), wxPoint(651, 278),
      wxPoint(684, 245), wxPoint(717, 212),  // 24,25,26,27
      wxPoint(750, 179), wxPoint(724, 143),
      wxPoint(694, 109), wxPoint(660, 79),  // 28,29,30,31
      wxPoint(624, 52),  wxPoint(591, 85),
      wxPoint(558, 118), wxPoint(525, 151),  // 32,33,34,35
      wxPoint(492, 184), wxPoint(444, 170),
      wxPoint(401, 166), wxPoint(355, 171),  // 36,37,38,39
      wxPoint(312, 184), wxPoint(279, 151),
      wxPoint(246, 118), wxPoint(213, 85),  // 40,41,42,43
      wxPoint(180, 52),  wxPoint(143, 78),
      wxPoint(109, 110), wxPoint(78, 144),  // 44,45,46,47
      wxPoint(52, 181),  wxPoint(85, 214),
      wxPoint(118, 247), wxPoint(151, 280),  // 48,49,50,51
      wxPoint(184, 313), wxPoint(171, 357),
      wxPoint(167, 404), wxPoint(172, 449),  // 52,53,54,55
      wxPoint(185, 491), wxPoint(152, 524),
      wxPoint(119, 557), wxPoint(86, 590),  // 56,57,58,59
      wxPoint(53, 623),  wxPoint(79, 660),
      wxPoint(109, 693), wxPoint(143, 724),  // 60,61,62,63
      wxPoint(261, 749), wxPoint(306, 749),
      wxPoint(351, 749), wxPoint(396, 749),  // blue home: 64, 65, 66, 67
      wxPoint(173, 675), wxPoint(169, 629),
      wxPoint(202, 596), wxPoint(235, 563),  // blue finish: 68, 69, 70, 71
      wxPoint(750, 541), wxPoint(750, 496),
      wxPoint(750, 451), wxPoint(750, 406),  // green home: 72, 73, 74, 75
      wxPoint(677, 629), wxPoint(630, 633),
      wxPoint(597, 600), wxPoint(564, 567),  // green finish: 76, 77, 78, 79
      wxPoint(542, 52),  wxPoint(497, 52),
      wxPoint(452, 52),  wxPoint(407, 52),  // red home: 80, 81, 82, 83
      wxPoint(629, 122), wxPoint(633, 169),
      wxPoint(600, 202), wxPoint(567, 235),  // red finish: 84, 85, 86, 87
      wxPoint(52, 263),  wxPoint(52, 308),
      wxPoint(52, 353),  wxPoint(52, 398),  // yellow home: 88, 89, 90, 91
      wxPoint(124, 175), wxPoint(171, 171),
      wxPoint(204, 204), wxPoint(237, 237),  // yellow finish: 92, 93, 94, 95
  };

  wxSize panelSize = panel->GetSize();
  boardMinX = 1000, boardMinY = 1000, boardMaxX = 0, boardMaxY = 0;
  for (const wxPoint& p : points) {
    boardMinX = std::min(boardMinX, p.x);
    boardMinY = std::min(boardMinY, p.y);
    boardMaxX = std::max(boardMaxX, p.x);
    boardMaxY = std::max(boardMaxY, p.y);
  }

  const double TARGET_BOARD_SIZE = 400.0;
  boardWidth = boardMaxX - boardMinX + 50;
  boardHeight = boardMaxY - boardMinY + 50;
  boardScaleFactor = TARGET_BOARD_SIZE / std::max(boardWidth, boardHeight);

  // Scale points based on panel size
  for (wxPoint& p : points) {
    p.x = static_cast<int>((p.x - boardMinX) * boardScaleFactor);
    p.y = static_cast<int>((p.y - boardMinY) * boardScaleFactor);
  }

  // Center the board on the panel
  int offsetX = (panelSize.GetWidth() - boardWidth * boardScaleFactor) / 2;
  int offsetY = (panelSize.GetHeight() - boardHeight * boardScaleFactor) / 4;
  for (wxPoint& p : points) {
    p.x += offsetX;
    p.y += offsetY;
  }

  boardSpots = points;

  // Rotate board for this client's POV
  int cwSteps =
      (4 - client->getPlayerIndex()) % 4;  // how many 90° clockwise steps

  // math coords: (0,0) at center
  // wx coords:   (0,0) at top left corner

  if (cwSteps != 0) {  // player 0: no rotation
    wxRect rect = GetBoardRect();
    double cx = rect.GetX() + rect.GetWidth() / 2.0;
    double cy = rect.GetY() + rect.GetHeight() / 2.0;

    for (wxPoint& p : boardSpots) {
      double dx = p.x - cx;
      double dy = p.y - cy;
      double rx = dx, ry = dy;

      switch (cwSteps) {
        case 1:  // player 3: 90° clockwise in math coords -> 3*90° clockwise
                 // in wx coords
          rx = dy;
          ry = -dx;
          break;
        case 2:  // player 2: 2*90° CW in MC -> 2*90° CW in WXC
          rx = -dx;
          ry = -dy;
          break;
        case 3:  // player 1: 3*90° CW in MC -> 90° CW in WXC
          rx = -dy;
          ry = dx;
          break;
      }

      p.x = static_cast<int>(std::round(cx + rx));
      p.y = static_cast<int>(std::round(cy + ry));
    }
  }

  // Define spot groups for different areas on the board
  spotGroups["blue_start"] = {0};
  spotGroups["blue_home"] = {64, 65, 66, 67};
  spotGroups["blue_finish"] = {68, 69, 70, 71};
  spotGroups["green_start"] = {16};
  spotGroups["green_home"] = {72, 73, 74, 75};
  spotGroups["green_finish"] = {76, 77, 78, 79};
  spotGroups["red_start"] = {32};
  spotGroups["red_home"] = {80, 81, 82, 83};
  spotGroups["red_finish"] = {84, 85, 86, 87};
  spotGroups["yellow_start"] = {48};
  spotGroups["yellow_home"] = {88, 89, 90, 91};
  spotGroups["yellow_finish"] = {92, 93, 94, 95};
  spotGroups["track"] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                         13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                         26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38,
                         39, 40, 41, 42, 43, 44, 45, 46, 47, 49, 50, 51,
                         52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

  // ===== CALCULATE ALL SCALED DIMENSIONS ONCE =====
  currentScaleFactor_ = boardScaleFactor;
  scaledSpotRadius_ = static_cast<int>(12.0 * boardScaleFactor);
  if (panelSize.GetWidth() <= MIN_WINDOW_WIDTH ||
      panelSize.GetHeight() <= MIN_WINDOW_HEIGHT) {
    scaledSpotRadius_ = static_cast<int>(scaledSpotRadius_ * 0.85);
  }
  scaledSpotRadius_ = std::max(10, std::min(scaledSpotRadius_, 15));

  scaledMarbleRadius_ = static_cast<int>(10.0 * boardScaleFactor);
  scaledMarbleRadius_ = std::max(8, std::min(scaledMarbleRadius_, 12));

  scaledIconSize_ = static_cast<int>(80.0 * boardScaleFactor);
  scaledIconSize_ = std::max(40, std::min(scaledIconSize_, 100));

  iconDistanceMultiplier_ = 1.35;

  // std::cout << "Spot radius: " << scaledSpotRadius_
  //           << ", Marble radius: " << scaledMarbleRadius_
  //           << ", Icon size: " << scaledIconSize_ << std::endl;
}

void MainGameFrame::RecenterBoard() {
  if (boardSpots.empty()) return;

  wxSize size = panel->GetSize();
  wxRect rect = GetBoardRect();

  // Old center (of the currently positioned board)
  int oldCenterX = rect.GetX() + rect.GetWidth() / 2;
  int oldCenterY = rect.GetY() + rect.GetHeight() / 2;

  // New center (25% above, 75% below)
  int newCenterX = size.GetWidth() / 2;
  double available = size.GetHeight() - rect.GetHeight();
  int newTop = static_cast<int>(available * 0.25);
  int newCenterY = newTop + rect.GetHeight() / 2;

  // Center the board on the panel
  int offsetX = newCenterX - oldCenterX;
  int offsetY = newCenterY - oldCenterY;
  for (wxPoint& p : boardSpots) {
    p.x += offsetX;
    p.y += offsetY;
  }

  CalculateUserIconPositions();
}

wxPoint MainGameFrame::getGroupCenter(const std::vector<int>& indices) const {
  double sumX = 0, sumY = 0;
  int count = 0;

  for (int idx : indices) {
    if (idx >= 0 && idx < (int)boardSpots.size()) {
      sumX += boardSpots[idx].x;
      sumY += boardSpots[idx].y;
      count++;
    }
  }

  if (count == 0) return wxPoint(0, 0);

  return wxPoint((int)(sumX / count), (int)(sumY / count));
}

int MainGameFrame::determineSlotFromPosition(const wxPoint& center,
                                             const wxPoint& p) const {
  int dx = p.x - center.x;
  int dy = p.y - center.y;

  if (dx < 0 && dy > 0) return 0;  // bottom-left
  if (dx > 0 && dy > 0) return 1;  // bottom-right
  if (dx > 0 && dy < 0) return 2;  // top-right
  return 3;                        // top-left
}

int MainGameFrame::getRotatedSlotForPlayer(int playerIndex) const {
  wxRect rect = GetBoardRect();
  wxPoint center(rect.GetX() + rect.GetWidth() / 2,
                 rect.GetY() + rect.GetHeight() / 2);

  std::string group;

  switch (playerIndex) {
    case 0:
      group = "blue_home";
      break;
    case 1:
      group = "green_home";
      break;
    case 2:
      group = "red_home";
      break;
    case 3:
      group = "yellow_home";
      break;
    default:
      return 0;
  }

  wxPoint pos = getGroupCenter(spotGroups.at(group));

  return determineSlotFromPosition(center, pos);
}

void MainGameFrame::CalculateUserIconPositions() {
  // Visual center of the rotated & scaled board
  wxRect rect = GetBoardRect();
  int centerX = rect.GetX() + rect.GetWidth() / 2;
  int centerY = rect.GetY() + rect.GetHeight() / 2;

  // Size of icon (relative to window size)
  int iconSize = scaledIconSize_;

  for (int i = 0; i < 4; ++i) {
    // Skip empty player slots
    if (client->getPlayerList()[i].name.empty()) {
      continue;
    }

    // Get this player's ACTUAL slot after board rotation
    int slot = getRotatedSlotForPlayer(i);

    // Home group for this player
    std::string groupName;

    switch (i) {
      case 0:
        groupName = "blue_home";
        break;
      case 1:
        groupName = "green_home";
        break;
      case 2:
        groupName = "red_home";
        break;
      case 3:
        groupName = "yellow_home";
        break;
    }

    // Compute the average home-field coordinate
    wxPoint target = getGroupCenter(spotGroups[groupName]);

    // Place the icon along the radial line using that slot
    PlaceIconAlongLine(i, slot, centerX, centerY, target, iconSize);
  }

  UpdatePlayerNameLabelsPositions(iconSize);
}

// place along line from center to target point
void MainGameFrame::PlaceIconAlongLine(int playerIndex, int slot, int centerX,
                                       int centerY, const wxPoint& targetPoint,
                                       int iconSize) {
  // center: center of the board
  // targetPoint: center of the player's home fields
  // dir: A vector from board center to the target
  //      (that tells you in which direction (from the center) that corner of
  //      the board is)
  float dirX = targetPoint.x - centerX;
  float dirY = targetPoint.y - centerY;

  // distance from the center to the target
  float length = std::sqrt(dirX * dirX + dirY * dirY);
  if (length == 0.0f) {
    // fallback, shouldn't really happen
    userIconPositions[playerIndex] = wxPoint(centerX, centerY);
    return;
  }

  // Unit direction from center -> home corner
  dirX /= length;
  dirY /= length;

  // Adjust factor to position the icon outside the board
  float distance = length * 1.1f;

  // Base position along the radial line
  float baseX = centerX + dirX * distance - iconSize / 2.0f;
  float baseY = centerY + dirY * distance - iconSize / 2.0f;

  // Per-slot offsets in screen pixels
  // WX coords:   (0,0) at top left corner
  //              +X = right, +Y = down
  int offsetX = 0;
  int offsetY = 0;

  switch (slot) {
    case 0:            // bottom-left (your own POV seat)
      offsetX = -150;  // left
      offsetY = -13;   // up
      break;
    case 1:           // bottom-right
      offsetX = -10;  // left
      offsetY = 150;  // down
      break;
    case 2:           // top-right
      offsetX = 153;  // right
      offsetY = 13;   // down
      break;
    case 3:            // top-left
      offsetX = 10;    // right
      offsetY = -150;  // up
      break;
  }

  userIconPositions[playerIndex] =
      wxPoint(static_cast<int>(std::round(baseX + offsetX)),
              static_cast<int>(std::round(baseY + offsetY)));
}

void MainGameFrame::testing_DebugUserIconPositions() {
  std::cout << "--testing user position--" << std::endl;
  for (int i = 0; i < 4; ++i) {
    std::cout << "player " << i << " position: x=" << userIconPositions[i].x
              << ", y=" << userIconPositions[i].y << std::endl;
  }
}

void MainGameFrame::UpdatePlayerNameLabelsPositions(int iconSize) {
  for (int player = 0; player < 4; ++player) {
    if (!playerNameLabels[player]) continue;

    // Skip empty player slots
    if (client->getPlayerList()[player].name.empty()) {
      playerNameLabels[player]->Hide();
      continue;
    }

    // iconPos: top-left of icon
    const wxPoint& iconPos = userIconPositions[player];

    // Get the label size to center it under the icon
    wxSize labelSize = playerNameLabels[player]->GetSize();

    // Get this player's ACTUAL slot after board rotation
    int slot = getRotatedSlotForPlayer(player);

    // Horizontal alignment: depending on player slot
    int labelX;
    switch (slot) {
      case 0:
        labelX = iconPos.x - labelSize.GetWidth();
        break;
      case 1:
        labelX = iconPos.x + iconSize;
        break;
      case 2:
        labelX = iconPos.x + iconSize;
        break;
      case 3:
        labelX = iconPos.x - labelSize.GetWidth();
        break;
    }

    // Place just below the icon (tweak "+ 10" to move closer/further)
    int labelY = iconPos.y + iconSize - labelSize.GetHeight() - 3;
    /* switch (slot) {
      case 0:
        labelY = iconPos.y + iconSize - labelSize.GetHeight() - 3;
        break;
      case 1:
        labelY = iconPos.y + iconSize - labelSize.GetHeight() - 3;
        break;
      case 2:
        labelY = iconPos.y + iconSize - labelSize.GetHeight() - 3;
        break;
      case 3:
        labelY = iconPos.y + iconSize - labelSize.GetHeight() - 3;
        break;
    } */

    playerNameLabels[player]->SetPosition(wxPoint(labelX, labelY));
    playerNameLabels[player]->Show();  // ensure visible
  }
}

void MainGameFrame::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(panel);
  DrawBoard(dc);
  DrawMarbles(dc);
  DrawCardHighlight(dc);
  DrawLastPlayedCard(dc);
}

void MainGameFrame::DrawBoard(wxDC& dc) {
  // paint background
  dc.SetBrush(wxBrush(wxColour(240, 212, 175)));
  dc.SetPen(wxPen(wxColour(240, 212, 175), 0));
  dc.DrawRectangle(0, 0, panel->GetSize().GetWidth(),
                   panel->GetSize().GetHeight());

  // define colors for different areas
  std::map<std::string, wxColour> colorMap = {
      {"yellow_home", wxColour(239, 189, 56)},
      {"yellow_finish", wxColour(239, 189, 56)},
      {"yellow_start", wxColour(239, 189, 56)},
      {"red_home", wxColour(204, 58, 49)},
      {"red_finish", wxColour(204, 58, 49)},
      {"red_start", wxColour(204, 58, 49)},
      {"green_home", wxColour(26, 89, 32)},
      {"green_finish", wxColour(26, 89, 32)},
      {"green_start", wxColour(26, 89, 32)},
      {"blue_home", wxColour(65, 86, 183)},
      {"blue_finish", wxColour(65, 86, 183)},
      {"blue_start", wxColour(65, 86, 183)},
      {"track", wxColour(182, 153, 113)}};

  // draw spots
  for (const auto& group : spotGroups) {
    const std::string& name = group.first;
    const wxColour color = colorMap[name];

    const bool isFinish = name.find("finish") != std::string::npos;
    const bool isStart = name.find("start") != std::string::npos;
    const bool isHome = name.find("home") != std::string::npos;
    const bool isTrack = name == "track";

    for (int idx : group.second) {
      if (idx >= static_cast<int>(boardSpots.size())) continue;

      if (isFinish || isStart || isHome) {
        // Outline only in the group's color
        dc.SetBrush(
            wxBrush(wxColour(182, 153, 113)));  // or wxTRANSPARENT_BRUSH
        dc.SetPen(wxPen(color, 3));
      } else if (isTrack) {
        dc.SetBrush(wxBrush(wxColour(182, 153, 113)));
        dc.SetPen(wxPen(wxColour(182, 153, 113), 1));
      } else {
        // Fallback (filled)
        dc.SetBrush(wxBrush(color));
      }

      dc.DrawCircle(boardSpots[idx], scaledSpotRadius_);
    }
  }

  // Use pre-calculated icon size
  int iconSize = scaledIconSize_;

  wxImage blueImg = blueUserBitmap.ConvertToImage().Rescale(
      iconSize, iconSize, wxIMAGE_QUALITY_HIGH);
  wxImage greenImg = greenUserBitmap.ConvertToImage().Rescale(
      iconSize, iconSize, wxIMAGE_QUALITY_HIGH);
  wxImage redImg = redUserBitmap.ConvertToImage().Rescale(iconSize, iconSize,
                                                          wxIMAGE_QUALITY_HIGH);
  wxImage yellowImg = yellowUserBitmap.ConvertToImage().Rescale(
      iconSize, iconSize, wxIMAGE_QUALITY_HIGH);
  wxImage greyImg = greyUserBitmap.ConvertToImage().Rescale(
      iconSize, iconSize, wxIMAGE_QUALITY_HIGH);

  wxBitmap playerBitmaps[4];
  playerBitmaps[0] = wxBitmap(blueImg);    // player 0
  playerBitmaps[1] = wxBitmap(greenImg);   // player 1
  playerBitmaps[2] = wxBitmap(redImg);     // player 2
  playerBitmaps[3] = wxBitmap(yellowImg);  // player 3
  wxBitmap greyBmp(greyImg);               // disconnected player

  for (int player = 0; player < 4; ++player) {
    bool disconnected =
        (std::find(disconnectedPlayers_.begin(), disconnectedPlayers_.end(),
                   player) != disconnectedPlayers_.end());

    // Skip empty player slots
    if (client->getPlayerList()[player].name.empty()) {
      continue;
    }
    const wxPoint& pos = userIconPositions[player];

    wxBitmap bmp = playerBitmaps[player];

    if (disconnected) bmp = greyBmp;

    dc.DrawBitmap(bmp, pos.x, pos.y, true);
  }

  // testing_DebugUserIconPositions();
}

std::string MainGameFrame::getPlayerDisplayName(size_t i) const {
  // Get the latest player list from the client
  auto players =
      client->getPlayerList();  // client->getPlayerList()[i].name.empty()

  if (i < players.size() && !players[i].name.empty()) {
    return players[i].name;
  }

  // Return for absent players
  std::cerr << "Player index " << i
            << " should not be called for display name because they are "
               "non-existent in GameState."
            << std::endl;
  return "";
}

wxPoint MainGameFrame::getPositionCenter(
    const BraendiDog::Position& pos) const {
  // TRACK
  if (pos.boardLocation == BraendiDog::BoardLocation::TRACK) {
    int spotIdx = static_cast<int>(pos.index);
    if (spotIdx >= 0 && spotIdx < static_cast<int>(boardSpots.size())) {
      return boardSpots[spotIdx];
    }
    return wxPoint(-1, -1);
  }

  // HOME&FINISH
  std::string groupName;
  bool isHome = (pos.boardLocation == BraendiDog::BoardLocation::HOME);

  switch (pos.playerID) {
    case 0:
      groupName = isHome ? "blue_home" : "blue_finish";
      break;
    case 1:
      groupName = isHome ? "green_home" : "green_finish";
      break;
    case 2:
      groupName = isHome ? "red_home" : "red_finish";
      break;
    case 3:
      groupName = isHome ? "yellow_home" : "yellow_finish";
      break;
    default:
      return wxPoint(-1, -1);
  }

  // get spot index within the group
  int posIndex = static_cast<int>(pos.index);
  auto it = spotGroups.find(groupName);

  if (it != spotGroups.end() && posIndex >= 0 &&
      posIndex < static_cast<int>(it->second.size())) {
    int spotIdx = it->second[posIndex];
    if (spotIdx >= 0 && spotIdx < static_cast<int>(boardSpots.size())) {
      return boardSpots[spotIdx];
    }
  }

  return wxPoint(-1, -1);
}
void MainGameFrame::DrawMarbles(wxDC& dc) {
  const wxColour playerMarbleColors[4] = {
      wxColour(65, 86, 183),  // blue (player 0)
      wxColour(26, 89, 32),   // green (player 1)
      wxColour(204, 58, 49),  // red (player 2)
      wxColour(239, 189, 56)  // yellow (player 3)
  };

  // highlight colors for players
  const wxColour playerHighlightColors[4] = {
      wxColour(100, 170, 255),  // soft blue
      wxColour(100, 170, 255), wxColour(100, 170, 255),
      wxColour(100, 170, 255)};

  const int marbleRadius = 10;

  // Use temp state if Seven move is in progress
  const auto& displayState =
      (moveController && moveController->getSevenTempState().has_value())
          ? moveController->getSevenTempState().value()
          : gameState_;

  const auto& players = displayState.getPlayers();

  // get possible destinations and selected marble from move controller
  std::vector<BraendiDog::Position> possibleDests;
  std::optional<BraendiDog::MarbleIdentifier> selectedMarble;

  if (moveController) {
    possibleDests = moveController->getPossibleDestinations();
    selectedMarble = moveController->getSelectedMarble();
  }

  // draw marbles
  for (size_t p = 0; p < players.size(); ++p) {
    if (!players[p].has_value()) continue;

    const auto& player = players[p].value();
    const auto& marbles = player.getMarbles();

    for (size_t m = 0; m < marbles.size(); ++m) {
      const auto& marblePos = marbles[m];
      wxPoint center = getPositionCenter(marblePos);

      if (center.x == -1) continue;

      // check if this marble is selected
      bool isSelected = false;
      if (selectedMarble.has_value()) {
        isSelected =
            (selectedMarble->playerID == p && selectedMarble->marbleIdx == m);
      }

      // draw marble
      if (isSelected) {
        // chosen marble with highlight
        dc.SetPen(wxPen(playerHighlightColors[p % 4], 3));
        dc.SetBrush(wxBrush(playerMarbleColors[p % 4]));
        dc.DrawCircle(center, marbleRadius);

        // outer highlight ring
        dc.SetPen(wxPen(playerHighlightColors[p % 4], 3));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawCircle(center, marbleRadius + 2);
      } else {
        // normal marble
        dc.SetPen(wxPen(wxColour(30, 30, 30), 1));
        dc.SetBrush(wxBrush(playerMarbleColors[p % 4]));
        dc.DrawCircle(center, marbleRadius);
      }
    }
  }

  // draw possible destinations highlight
  if (!possibleDests.empty() && selectedMarble.has_value()) {
    // highlight color based on selected marble's player
    size_t highlightPlayerID = selectedMarble->playerID;
    wxColour highlightColor = playerHighlightColors[highlightPlayerID % 4];

    for (const auto& dest : possibleDests) {
      wxPoint center = getPositionCenter(dest);
      if (center.x == -1) continue;

      // outer ring
      dc.SetPen(wxPen(highlightColor, 4));
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawCircle(center, marbleRadius);

      // // inner ring
      // dc.SetPen(wxPen(highlightColor, 2));
      // dc.DrawCircle(center, marbleRadius + 3);

      // center dot
      dc.SetPen(*wxTRANSPARENT_PEN);
      wxColour transparentHighlight(
          highlightColor.Red(), highlightColor.Green(), highlightColor.Blue(),
          180);  // alpha = 180
      dc.SetBrush(wxBrush(transparentHighlight));
      dc.DrawCircle(center, 4);
    }
  }
}

void MainGameFrame::DrawCardHighlight(wxDC& dc) {
  if (highlightedCardIndex < 0 ||
      highlightedCardIndex >= (int)playerHandCards.size())
    return;

  wxStaticBitmap* card = playerHandCards[highlightedCardIndex];
  wxPoint pos = card->GetPosition();
  wxSize size = card->GetSize();

  dc.SetPen(wxPen(wxColour(100, 170, 255), 4));  // soft blue outline
  dc.SetBrush(*wxTRANSPARENT_BRUSH);

  // draw rectangle around the card
  dc.DrawRoundedRectangle(pos.x - 2, pos.y - 2, size.GetWidth() + 4,
                          size.GetHeight() + 4, 6);
}

void MainGameFrame::DrawLastPlayedCard(wxDC& dc) {
  if (!gameState_.getLastPlayedCard().has_value()) return;

  // Load card bitmap
  int cardImageIndex =
      getCardImageIndex(gameState_.getLastPlayedCard().value());
  wxString imagePath =
      wxString::Format("../assets/cards/Poker_%d.png", cardImageIndex);

  wxBitmap bitmap(imagePath, wxBITMAP_TYPE_PNG);
  if (!bitmap.IsOk()) return;

  // Rescale
  int cardW = 90;
  int cardH = 135;

  wxImage img = bitmap.ConvertToImage();
  img.Rescale(cardW, cardH, wxIMAGE_QUALITY_HIGH);
  bitmap = wxBitmap(img);

  // Center of board
  wxRect rect = GetBoardRect();
  int cx = rect.GetX() + rect.GetWidth() / 2 - cardW / 2;
  int cy = rect.GetY() + rect.GetHeight() / 2 - cardH / 2;

  // Draw the card itself
  dc.DrawBitmap(bitmap, cx, cy, true);
}

void MainGameFrame::UpdatePlayerHand() {
  // clear existing cards
  for (auto card : playerHandCards) {
    card->Destroy();
  }
  playerHandCards.clear();

  wxSize size = panel->GetSize();
  int playerCardHeight = 110;
  int playerCardWidth = playerCardHeight / 3 * 2;  // maintain aspect ratio

  const auto& myHand =
      gameState_.getPlayerByIndex(client->getPlayerIndex()).value().getHand();
  int numPlayerCards = myHand.size();
  int spacing = 20;

  int availableWidth = size.GetWidth();

  int startX = (availableWidth - (numPlayerCards * playerCardWidth +
                                  (numPlayerCards - 1) * spacing)) /
               2;
  int bottomMargin = 40;

  // create card images
  for (size_t i = 0; i < myHand.size(); i++) {
    size_t imageIndex = getCardImageIndex(myHand[i]);
    wxString imagePath =
        wxString::Format("../assets/cards/Poker_%zu.png", imageIndex);

    wxBitmap bitmap(imagePath, wxBITMAP_TYPE_ANY);
    if (!bitmap.IsOk()) {
      std::cout << "Failed to load card image: " << imagePath << std::endl;
      continue;
    }
    int origWidth = bitmap.GetWidth();
    int origHeight = bitmap.GetHeight();

    // convert the image into desired size
    if (origWidth > 0 && origHeight > 0) {
      wxImage img = bitmap.ConvertToImage();
      img.Rescale(playerCardWidth, playerCardHeight, wxIMAGE_QUALITY_HIGH);
      bitmap = wxBitmap(img);
    }

    // wxImage img = bitmap.ConvertToImage();
    // img.Rescale(playerCardWidth, playerCardHeight, wxIMAGE_QUALITY_HIGH);
    // bitmap = wxBitmap(img);

    int xPos = startX + i * (playerCardWidth + spacing);
    int yPos = size.GetHeight() - playerCardHeight - bottomMargin;

    auto image =
        new wxStaticBitmap(panel, wxID_ANY, bitmap, wxPoint(xPos, yPos),
                           wxSize(playerCardWidth, playerCardHeight));

    // bind click event
    image->Bind(wxEVT_LEFT_DOWN,
                [this, i](wxMouseEvent& event) { OnHandCardClicked(i); });

    playerHandCards.push_back(image);
  }

  panel->Refresh();
}

void MainGameFrame::UpdateDiceIcon(int cardCount) {
  if (!diceIcon) return;
  int index = std::max(1, std::min(6, cardCount)) - 1;

  if (diceImages[index].IsOk()) {
    diceIcon->SetBitmap(diceImages[index]);
    diceIcon->SetToolTip(
        wxString::Format("Current deal: %d cards per player", index + 1));
    diceIcon->Refresh();
  }
}

void MainGameFrame::OnMarbleClicked(wxMouseEvent& event) {
  wxPoint clickPos = event.GetPosition();

  for (size_t i = 0; i < boardSpots.size(); i++) {
    wxPoint pos = boardSpots[i];

    double distance = std::sqrt(std::pow(clickPos.x - pos.x, 2) +
                                std::pow(clickPos.y - pos.y, 2));

    if (distance <= scaledSpotRadius_ + 2) {
      if (!moveController) return;

      // Convert spotIndex to proper Position
      BraendiDog::Position position = spotIndexToPosition(i);
      std::cout << "Spot clicked: " << i << " at position: ("
                << static_cast<int>(position.boardLocation) << ", "
                << position.index << ", " << position.playerID << ")"
                << std::endl;
      moveController->onBoardPositionClicked(position);

      // Marble click handled, exit loop
      break;
    }
  }

  event.Skip();
}

void MainGameFrame::OnServerUpdate(wxThreadEvent& event) {
  try {
    nlohmann::json messageJson =
        nlohmann::json::parse(event.GetString().ToStdString());

    auto message = Message::fromJson(messageJson);

    std::cout << "Received message type: "
              << static_cast<int>(message->getMessageType()) << std::endl;

    switch (message->getMessageType()) {
      /// GAME STATE UPDATE///
      case MessageType::BRDC_GAMESTATE_UPDATE: {
        auto* gsMsg = static_cast<GameStateUpdateMessage*>(message.get());
        const BraendiDog::GameState& gs = gsMsg->gameState;

        // Update local game state
        // check if hand is empty before updating
        // or if player does not exist yet because game just started
        if (!gameState_.getPlayerByIndex(client->getPlayerIndex())
                 .has_value() ||
            gameState_.getPlayerByIndex(client->getPlayerIndex())
                .value()
                .isHandEmpty()) {
          gameState_ = gs;
          std::cout << "GameState updated, hand was empty." << std::endl;
        } else {
          // take hand from existing gamestate
          auto hand = gameState_.getPlayerByIndex(client->getPlayerIndex())
                          .value()
                          .getHand();
          gameState_ = gs;
          gameState_.getPlayerByIndex(client->getPlayerIndex())
              .value()
              .setHand(hand);
          std::cout << "GameState updated, hand preserved." << std::endl;
        }

        panel->Refresh();

        UpdateGamestate();

        // Check if game ended
        if (gameState_.checkGameEnd()) {
          // Rest gets handled by BRDC_PLAYER_FINISHED and BRDC_RESULTS message
          return;
        }

        if (gameState_.isMyTurn(client->getPlayerIndex())) {
          statusText->SetLabel("It's your turn!");
          takeTurn();
        } else {
          statusText->SetLabel("Waiting for other players to move...");
        }

        int roundCardCount = gs.getRoundCardCount();
        UpdateDiceIcon(roundCardCount);
        break;
      }
      /// PRIVATE CARDS DEALT ///
      case MessageType::PRIV_CARDS_DEALT: {
        auto* dealt = static_cast<CardsDealtMessage*>(message.get());
        if (dealt->getPlayerId() ==
            static_cast<size_t>(client->getPlayerIndex())) {
          auto& playerOpt =
              gameState_.getPlayerByIndex(client->getPlayerIndex());
          playerOpt.value().setHand(dealt->cards);
          UpdatePlayerHand();
          highlightedCardIndex = -1;
          if (statusText) {
            statusText->SetLabel("Your hand received.");
          }
          if (gameState_.isMyTurn(client->getPlayerIndex())) {
            statusText->SetLabel("It's your turn!");
            takeTurn();
          } else {
            statusText->SetLabel("Waiting for other players to move...");
          }
        } else {
          std::cerr << "Received PRIV_CARDS_DEALT for another player: "
                    << dealt->getPlayerId() << std::endl
                    << std::flush;
        }
        break;
      }
      /// PLAY CARD RESPONSE ///
      case MessageType::RESP_PLAY_CARD: {
        auto* resp = static_cast<PlayCardResponseMessage*>(message.get());
        if (resp->getSuccess()) {
          statusText->SetLabel("Card played successfully.");
          // Update pop card in local copy of gamestate
          auto& playerOpt =
              gameState_.getPlayerByIndex(client->getPlayerIndex());
          playerOpt.value().popCardFromHand(resp->handIndex);
          UpdatePlayerHand();

          // Update legal moves in move controller
          moveController->setLegalMoves({});  // reset legal moves
          moveController->clearSelection();
          highlightedCardIndex = -1;

          panel->Refresh();
        } else {
          statusText->SetLabel(resp->getErrorMsg());
          highlightedCardIndex = -1;
          panel->Refresh();
        }
        break;
      }
      /// FOLD TURN RESPONSE ///
      case MessageType::RESP_SKIP_TURN: {
        auto* resp = static_cast<SkipTurnResponseMessage*>(message.get());
        if (resp->getSuccess()) {
          statusText->SetLabel("Forced to fold for the round.");

          auto& playerOpt =
              gameState_.getPlayerByIndex(client->getPlayerIndex());
          playerOpt.value().setHand({});  // clear hand on fold
          UpdatePlayerHand();
          highlightedCardIndex = -1;

          // Update legal moves in move controller
          moveController->setLegalMoves({});  // reset legal moves
          moveController->clearSelection();
        } else {
          statusText->SetLabel("Invalid fold request: " + resp->getErrorMsg());
        }
        break;
      }
      /// BROADCAST if PLAYER FINISHED ///
      case MessageType::BRDC_PLAYER_FINISHED: {
        auto* finMsg = static_cast<PlayerFinishedMessage*>(message.get());
        const size_t pId = finMsg->playerId;
        std::string playerName = getPlayerDisplayName(pId);
        statusText->SetLabel(playerName + " has finished the game!");
        if (pId == static_cast<size_t>(client->getPlayerIndex())) {
          // Update pop card in local copy of gamestate
          auto& playerOpt = gameState_.getPlayerByIndex(pId);
          playerOpt.value().setHand({});  // clear hand on finish
          UpdatePlayerHand();

          // Update legal moves in move controller
          moveController->setLegalMoves({});  // reset legal moves
          moveController->clearSelection();
          highlightedCardIndex = -1;

          panel->Refresh();
        }
        // TODO: Show some visual indication if client is the finisher and if
        // not maybe indicate finished visually on player icon
        break;
      }
      /// GAME END RESULTS ///
      case MessageType::BRDC_RESULTS: {
        auto* resMsg = static_cast<GameResultsMessage*>(message.get());
        statusText->SetLabel("Game Over!");
        auto& playerOpt = gameState_.getPlayerByIndex(client->getPlayerIndex());
        if (playerOpt.has_value()) playerOpt.value().setHand({});
        UpdatePlayerHand();
        moveController->setLegalMoves({});
        moveController->clearSelection();
        panel->Refresh();
        ShowResults(resMsg->rankings);
        break;
      }
      /// DISCONNECTION DETECTION ///
      case MessageType::BRDC_PLAYER_DISCONNECTED: {
        auto* discMsg = static_cast<PlayerDisconnectedMessage*>(message.get());
        int id = static_cast<int>(discMsg->playerId);

        if (id >= 0 && id <= 3) {
          if (std::find(disconnectedPlayers_.begin(),
                        disconnectedPlayers_.end(),
                        id) == disconnectedPlayers_.end()) {
            disconnectedPlayers_.push_back(id);
          }

          // CalculateUserIconPositions();
          UpdateGamestate();
          panel->Refresh();
        }
        break;
      }
      default: {
        std::cout << "Unhandled message type: "
                  << static_cast<int>(message->getMessageType()) << std::endl;
        break;
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Error processing server message: " << e.what() << std::endl;
    statusText->SetLabel("Error: " + std::string(e.what()));
  }
}

void MainGameFrame::UpdateGamestate() {
  // Hide placeholder text when game starts
  if (placeholderText) {
    placeholderText->Hide();
  }

  // Update status text
  statusText->SetLabel("Game in progress - Brändi Dog Board");

  // Update player info text
  std::string playerInfo = "Player information: ";
  int printed = 0;
  for (size_t i = 0; i < 4; i++) {
    // Skip empty player slots
    if (client->getPlayerList()[i].name.empty()) {
      continue;
    }
    if (printed > 0) {
      playerInfo += " | ";
    }
    playerInfo += getPlayerDisplayName(i);
    if (i == static_cast<size_t>(client->getPlayerIndex())) {
      playerInfo += " (You)";
    }
    printed++;
  }
  playerInfoText->SetLabel(playerInfo);

  // Update player name labels
  for (size_t i = 0; i < 4; i++) {
    bool disconnected =
        (std::find(disconnectedPlayers_.begin(), disconnectedPlayers_.end(),
                   i) != disconnectedPlayers_.end());

    // Skip empty player slots
    if (client->getPlayerList()[i].name.empty()) {
      continue;
    }
    std::string name = getPlayerDisplayName(i);

    if (!playerNameLabels[i]) continue;

    if (disconnected) {
      name += " (Disconnected)";
      playerNameLabels[i]->SetForegroundColour(wxColour(120, 120, 120));
    } else {
      playerNameLabels[i]->SetForegroundColour(wxColour(0, 0, 0));
    }

    wxString playerName = wxString::FromUTF8(name.c_str());

    bool isMe = (i == static_cast<size_t>(client->getPlayerIndex()));
    bool isCurrent = (i == gameState_.getCurrentPlayer());

    // Build label text
    if (isCurrent) {
      if (isMe) {
        playerName = "Your turn!";
      } else {
        playerName += "'s turn!";
      }
    }

    if (playerNameLabels[i]) {
      playerNameLabels[i]->SetLabel(playerName);

      // Highlight current player's turn
      wxFont f = playerNameLabels[i]->GetFont();
      if (isCurrent)
        f.SetWeight(wxFONTWEIGHT_BOLD);
      else
        f.SetWeight(wxFONTWEIGHT_NORMAL);

      playerNameLabels[i]->SetFont(f);

      playerNameLabels[i]->Refresh();
    }

    playerNameLabels[i]->Refresh();
  }

  panel->Refresh();
  UpdatePlayerNameLabelsPositions(scaledIconSize_);
}

void MainGameFrame::takeTurn() const {
  // Add turn check for safety
  if (!gameState_.isMyTurn(client->getPlayerIndex())) {
    return;
  }

  // At round start waits for priv cards deal call
  if (gameState_.getPlayerByIndex(client->getPlayerIndex())
          .value()
          .isHandEmpty()) {
    return;
  }

  // Compute possible moves
  std::vector<BraendiDog::Move> possibleMoves_ = gameState_.computeLegalMoves();
  // Check for normal moves and special moves
  if (possibleMoves_.empty() && !gameState_.hasSpecialMoves().first &&
      !gameState_.hasSpecialMoves().second) {
    std::cout << "No legal moves available." << std::endl;
    moveController->foldTurn();
    return;
  }

  // Debug output of possible special moves
  // Add Joker and Seven check output
  if (gameState_.getPlayerByIndex(client->getPlayerIndex())
          .value()
          .hasJokerInHand()) {
    // Joker move handling is done in move controller
    std::cout << "Joker detected in hand." << std::endl;
  } else if (gameState_.getPlayerByIndex(client->getPlayerIndex())
                 .value()
                 .hasCardInHand(7)) {
    // Seven move handling is done in move controller
    std::cout << "Seven detected in hand." << std::endl;
  }

  // Update legal moves in move controller -> only normal moves here
  moveController->setLegalMoves(std::move(possibleMoves_));
}

void MainGameFrame::UpdatePositions() {
  // Update status text position
  statusText->SetPosition(wxPoint(10, 10));

  // Hide placeholder text if exists
  if (placeholderText) {
    placeholderText->Hide();
  }

  // Update player info position at bottom of screen
  wxSize size = this->GetClientSize();
  int newY = size.GetHeight() - 30;
  playerInfoText->SetPosition(wxPoint(10, newY));
  wxRect boardRect = GetBoardRect();

  // Update rules button position at right corner
  if (rulesButton) {
    wxSize panelSize = panel->GetSize();
    wxSize iconSize = rulesButton->GetSize();
    rulesButton->SetPosition(
        wxPoint(boardRect.GetRight() + 90,
                boardRect.GetY() + boardRect.GetHeight() / 2 + 40));
  }
  if (diceIcon) {
    wxSize panelSize = panel->GetSize();
    wxSize diceSize = diceIcon->GetSize();
    int centerY = panelSize.GetHeight() / 2;
    diceIcon->SetPosition(
        wxPoint(boardRect.GetRight() + 90,
                boardRect.GetY() + boardRect.GetHeight() / 2 - 30));
  }
}

void MainGameFrame::OnResize(wxSizeEvent& event) {
  // Board is already initialized in constructor
  // This only handles user-initiated window resizing
  wxSize clientSize = this->GetClientSize();
  panel->SetSize(clientSize);

  CallAfter([this]() {
    RecenterBoard();
    UpdatePositions();
    UpdatePlayerHand();
    panel->Refresh();
  });
}

void MainGameFrame::OnHandCardClicked(size_t cardIndex) {
  // First check for Special Cards
  // Joker card handling
  if (IsJokerCard(static_cast<int>(cardIndex))) {
    // Show the Joker selection popup
    if (!ShowJokerSelectionPopup(static_cast<int>(cardIndex))) {
      // User canceled, do nothing
      return;
    }
    // User selected a rank, continue with normal processing
  }
  // Seven card handling
  else if (IsSevenCard(static_cast<int>(cardIndex))) {
    size_t cardID = gameState_.getPlayerByIndex(client->getPlayerIndex())
                        .value()
                        .getHand()[cardIndex];
    std::vector<BraendiDog::Move> possibleMoves_ = gameState_.computeLegalMoves(
        std::array<size_t, 3>{cardID, cardIndex, cardID}, true);
    if (possibleMoves_.empty()) {
      std::cout << "No legal moves available for Seven card." << std::endl;
      statusText->SetLabel("No legal moves available for Seven card.");
      return;
    }
    moveController->setSevenMoves(std::move(possibleMoves_));
  }

  // Existing card click handling code
  size_t cardValue;
  if (IsJokerCard(static_cast<int>(cardIndex)))
    cardValue = 14;  // Joker
  else {
    const auto& playerOpt =
        gameState_.getPlayerByIndex(client->getPlayerIndex());
    if (playerOpt.has_value()) {
      const auto& hand = playerOpt.value().getHand();
      cardValue = hand[cardIndex] % 13 + 1;  // 1-13
    }
  }

  std::cout << "Card clicked: " << cardIndex << " with value: " << cardValue
            << std::endl;

  // DEBUG: std::nullopt (empty), -1 (disconn.), 0 (lost/only player)
  // std::array<std::optional<int>, 4> fakeLeaderboard = {1, 3, 2, 0};
  // ShowResults(fakeLeaderboard);

  if (!moveController) return;
  moveController->onCardClicked(static_cast<int>(cardIndex));

  // Read the updated selection
  highlightedCardIndex = moveController->getSelectedHandIndex();
  // Redraw panel including highlight
  panel->Refresh();
}

void MainGameFrame::OnRulesButtonClicked(wxCommandEvent& event) {
  wxDialog* rulesDialog =
      new wxDialog(this, wxID_ANY, "Brändi Dog Rules", wxDefaultPosition,
                   wxSize(800, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
  rulesDialog->SetBackgroundColour(wxColour(203, 163, 110));

  if (!rulesImageBitmap.IsOk()) {
    rulesImageBitmap = wxBitmap("../assets/Rules.png", wxBITMAP_TYPE_PNG);
  }

  if (rulesImageBitmap.IsOk()) {
    wxScrolledWindow* scrolledWindow =
        new wxScrolledWindow(rulesDialog, wxID_ANY);

    int imgWidth = rulesImageBitmap.GetWidth();
    int imgHeight = rulesImageBitmap.GetHeight();

    scrolledWindow->SetVirtualSize(imgWidth, imgHeight);
    scrolledWindow->SetScrollRate(10, 10);

    wxStaticBitmap* rulesImage = new wxStaticBitmap(
        scrolledWindow, wxID_ANY, rulesImageBitmap, wxPoint(0, 0));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(scrolledWindow, 1, wxEXPAND | wxALL, 5);

    wxButton* closeButton = new wxButton(rulesDialog, wxID_CLOSE, "Close");
    closeButton->Bind(wxEVT_BUTTON, [rulesDialog](wxCommandEvent&) {
      rulesDialog->EndModal(wxID_CLOSE);
    });

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(closeButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    rulesDialog->SetSizer(mainSizer);
  } else {
    wxMessageBox("Unable to load rules image.", "Error", wxICON_ERROR);
  }

  rulesDialog->ShowModal();
  rulesDialog->Destroy();
}

BraendiDog::Position MainGameFrame::spotIndexToPosition(int spotIndex) const {
  // Check if it's in blue_home
  if (std::find(spotGroups.at("blue_home").begin(),
                spotGroups.at("blue_home").end(),
                spotIndex) != spotGroups.at("blue_home").end()) {
    size_t homeIndex = spotIndex - 64;  // 64-67 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::HOME, homeIndex, 0);
  }

  // Check if it's in blue_finish (FINISH)
  if (std::find(spotGroups.at("blue_finish").begin(),
                spotGroups.at("blue_finish").end(),
                spotIndex) != spotGroups.at("blue_finish").end()) {
    size_t FINISHIndex = spotIndex - 68;  // 68-71 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::FINISH, FINISHIndex,
                                0);
  }

  // Check if it's in green_home
  if (std::find(spotGroups.at("green_home").begin(),
                spotGroups.at("green_home").end(),
                spotIndex) != spotGroups.at("green_home").end()) {
    size_t homeIndex = spotIndex - 72;  // 72-75 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::HOME, homeIndex, 1);
  }

  // Check if it's in green_finish (FINISH)
  if (std::find(spotGroups.at("green_finish").begin(),
                spotGroups.at("green_finish").end(),
                spotIndex) != spotGroups.at("green_finish").end()) {
    size_t FINISHIndex = spotIndex - 76;  // 76-79 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::FINISH, FINISHIndex,
                                1);
  }

  // Check if it's in red_home
  if (std::find(spotGroups.at("red_home").begin(),
                spotGroups.at("red_home").end(),
                spotIndex) != spotGroups.at("red_home").end()) {
    size_t homeIndex = spotIndex - 80;  // 80-83 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::HOME, homeIndex, 2);
  }

  // Check if it's in red_finish (FINISH)
  if (std::find(spotGroups.at("red_finish").begin(),
                spotGroups.at("red_finish").end(),
                spotIndex) != spotGroups.at("red_finish").end()) {
    size_t FINISHIndex = spotIndex - 84;  // 84-87 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::FINISH, FINISHIndex,
                                2);
  }

  // Check if it's in yellow_home
  if (std::find(spotGroups.at("yellow_home").begin(),
                spotGroups.at("yellow_home").end(),
                spotIndex) != spotGroups.at("yellow_home").end()) {
    size_t homeIndex = spotIndex - 88;  // 88-91 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::HOME, homeIndex, 3);
  }

  // Check if it's in yellow_finish (FINISH)
  if (std::find(spotGroups.at("yellow_finish").begin(),
                spotGroups.at("yellow_finish").end(),
                spotIndex) != spotGroups.at("yellow_finish").end()) {
    size_t FINISHIndex = spotIndex - 92;  // 92-95 -> 0-3
    return BraendiDog::Position(BraendiDog::BoardLocation::FINISH, FINISHIndex,
                                3);
  }

  // Otherwise it's TRACK (includes track + start positions)
  // For TRACK, the index is just the spotIndex itself
  return BraendiDog::Position(BraendiDog::BoardLocation::TRACK,
                              static_cast<size_t>(spotIndex),
                              0);  // playerID doesn't matter for track
}

bool MainGameFrame::IsJokerCard(int handIndex) const {
  const auto& playerOpt = gameState_.getPlayerByIndex(client->getPlayerIndex());
  if (!playerOpt.has_value()) {
    return false;
  }

  const auto& hand = playerOpt.value().getHand();
  if (static_cast<size_t>(handIndex) >= hand.size()) {
    return false;
  }

  // Card IDs 52 and 53 are jokers
  size_t cardId = hand[handIndex];
  return (cardId == 52 || cardId == 53);
}

bool MainGameFrame::IsSevenCard(int handIndex) const {
  const auto& playerOpt = gameState_.getPlayerByIndex(client->getPlayerIndex());
  if (!playerOpt.has_value()) {
    return false;
  }

  const auto& hand = playerOpt.value().getHand();
  if (static_cast<size_t>(handIndex) >= hand.size()) {
    return false;
  }

  size_t cardId = hand[handIndex];
  size_t rank = cardId % 13 + 1;  // 1-13
  return (rank == 7);
}

bool MainGameFrame::ShowJokerSelectionPopup(int handIndex) {
  // Create a dialog for the Joker selection
  wxDialog* jokerDialog =
      new wxDialog(this, wxID_ANY, "Joker", wxDefaultPosition, wxSize(700, 350),
                   wxDEFAULT_DIALOG_STYLE);
  jokerDialog->SetBackgroundColour(wxColour(203, 163, 110));

  // Create a scrolled window to hold the card options
  wxScrolledWindow* scrollPanel = new wxScrolledWindow(jokerDialog, wxID_ANY);
  wxBoxSizer* cardSizer = new wxBoxSizer(wxHORIZONTAL);

  // Card dimensions
  int cardWidth = 70;
  int cardHeight = 105;

  // Track which card rank was selected
  int selectedRank = -1;

  // Create an option for each card rank (Ace through King)
  for (int rank = 1; rank <= 13; rank++) {
    // Load the card image
    wxString imagePath = wxString::Format("../assets/cards/Poker_%d.png", rank);
    wxBitmap bitmap(imagePath, wxBITMAP_TYPE_PNG);

    if (bitmap.IsOk()) {
      // Scale the image
      wxImage img = bitmap.ConvertToImage();
      img.Rescale(cardWidth, cardHeight, wxIMAGE_QUALITY_HIGH);
      bitmap = wxBitmap(img);

      // Create a panel for this card
      wxPanel* cardPanel = new wxPanel(scrollPanel, wxID_ANY, wxDefaultPosition,
                                       wxSize(cardWidth, cardHeight));

      // Store the rank in the panel's client data
      cardPanel->SetClientData(
          reinterpret_cast<void*>(static_cast<intptr_t>(rank)));

      // Paint the card on the panel
      cardPanel->Bind(wxEVT_PAINT, [bitmap](wxPaintEvent& evt) {
        wxPaintDC dc(static_cast<wxPanel*>(evt.GetEventObject()));
        dc.DrawBitmap(bitmap, 0, 0, true);
      });

      // Handle click events
      cardPanel->Bind(wxEVT_LEFT_DOWN, [&selectedRank, jokerDialog,
                                        cardPanel](wxMouseEvent& evt) {
        // Get the rank from client data
        selectedRank = static_cast<int>(
            reinterpret_cast<intptr_t>(cardPanel->GetClientData()));
        jokerDialog->EndModal(wxID_OK);
      });

      // Add margins around each card
      cardSizer->Add(cardPanel, 0, wxALL, 5);
    }
  }

  scrollPanel->SetSizer(cardSizer);
  scrollPanel->FitInside();
  scrollPanel->SetScrollRate(10, 10);

  // Add the scrolled panel to the main dialog
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

  // Add instruction text
  wxStaticText* instructionText = new wxStaticText(
      jokerDialog, wxID_ANY, "Select which card the Joker should mimic:");
  mainSizer->Add(instructionText, 0, wxALL | wxALIGN_CENTER, 10);

  // Add the card panel
  mainSizer->Add(scrollPanel, 1, wxEXPAND | wxALL, 5);

  // Add cancel button
  wxButton* cancelButton = new wxButton(jokerDialog, wxID_CANCEL, "Cancel");
  mainSizer->Add(cancelButton, 0, wxALL | wxALIGN_CENTER, 10);

  jokerDialog->SetSizer(mainSizer);

  // Show the dialog and wait for user input
  int result = jokerDialog->ShowModal();

  // Process the result
  if (result == wxID_OK && selectedRank != -1) {
    if (moveController) {
      // Pass the selected rank to the move controller
      moveController->setJokerRank(selectedRank, handIndex);
      std::cout << "Joker will be played as rank: " << selectedRank
                << std::endl;
      if (selectedRank == 7) {
        // Seven card handling
        size_t syntheticCardID = (selectedRank - 1);  // 0-12 for Ace-King
        size_t cardID = gameState_.getPlayerByIndex(client->getPlayerIndex())
                            .value()
                            .getHand()[handIndex];
        std::vector<BraendiDog::Move> possibleMoves_ =
            gameState_.computeLegalMoves(
                std::array<size_t, 3>{syntheticCardID,
                                      static_cast<size_t>(handIndex), cardID},
                true);
        if (possibleMoves_.empty()) {
          std::cout << "No legal moves available for Seven card." << std::endl;
          statusText->SetLabel("No legal moves available for Seven card.");
          return false;
        }

        moveController->setSevenMoves(std::move(possibleMoves_));
      }
    }

    jokerDialog->Destroy();
    return true;
  }

  if (moveController) {
    moveController->clearJokerRank();
  }

  jokerDialog->Destroy();
  return false;
}

void MainGameFrame::ShowResults(
    const std::array<std::optional<int>, 4>& leaderboard) {
  wxDialog* dlg =
      new wxDialog(this, wxID_ANY, "Game Over", wxDefaultPosition,
                   wxSize(400, 350), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
  dlg->SetBackgroundColour(wxColour(203, 163, 110));

  // Initialization
  wxFont titleFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                   wxFONTWEIGHT_BOLD);
  wxFont nameFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_BOLD);
  wxFont statusFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC,
                    wxFONTWEIGHT_BOLD);
  const int iconSize = 50;
  wxSize panelSize(iconSize, iconSize);
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* listSizer = new wxBoxSizer(wxVERTICAL);

  // Grid: [ Rank | Name | Status ]
  wxFlexGridSizer* grid = new wxFlexGridSizer(0, 3, 6, 10);
  grid->AddGrowableCol(1, 1);  // name column can expand

  auto MakeRankCell = [&](wxWindow* child) -> wxWindow* {
    wxPanel* p = new wxPanel(dlg, wxID_ANY, wxDefaultPosition, panelSize);
    p->SetBackgroundColour(dlg->GetBackgroundColour());
    p->SetMinSize(panelSize);

    if (child) {
      child->Reparent(p);
      child->SetMinSize(wxSize(iconSize, iconSize));
      child->SetSize(wxSize(iconSize, iconSize));
      child->Centre();
    }
    return p;
  };

  // Groups
  std::vector<std::pair<int, int>> finishedPlayers;  // won: (rank, id)
  int unfinishedPlayer = -2;             // lost/only player left: id
  std::vector<int> disconnectedPlayers;  // disconnected: id

  for (int player = 0; player < 4; ++player) {
    if (!leaderboard[player].has_value()) continue;  // seat was never used

    int val = leaderboard[player].value();
    if (val > 0) {
      finishedPlayers.emplace_back(val, player);
    } else if (val == 0) {
      unfinishedPlayer = player;
    } else {
      disconnectedPlayers.push_back(player);
    }
  }

  // Title
  wxString titleStr = "Results";
  int value = leaderboard[client->getPlayerIndex()].value();
  if (value > 0) {
    titleStr = "Congratulations!";
  } else if (value == 0 && !finishedPlayers.empty()) {
    titleStr = "You Lost";
  }
  wxStaticText* title = new wxStaticText(dlg, wxID_ANY, titleStr);
  title->SetFont(titleFont);
  mainSizer->Add(title, 0, wxALIGN_CENTER | wxALL, 20);

  // Display finished players
  if (!finishedPlayers.empty()) {
    // Sort by rank
    std::sort(finishedPlayers.begin(), finishedPlayers.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& fp : finishedPlayers) {
      int rank = fp.first;
      int finishedPlayer = fp.second;

      std::string name = getPlayerDisplayName(finishedPlayer);
      if (name.empty()) {
        name = "Player";
      }

      // Column 1: rank
      wxWindow* rankWidget = nullptr;
      if (rank >= 1 && rank <= 3 && rankBitmaps[rank - 1].IsOk()) {
        wxImage img = rankBitmaps[rank - 1].ConvertToImage();
        img.Rescale(iconSize, iconSize, wxIMAGE_QUALITY_HIGH);

        wxPanel* iconPanel =
            new wxPanel(dlg, wxID_ANY, wxDefaultPosition, panelSize);
        iconPanel->SetBackgroundColour(dlg->GetBackgroundColour());
        iconPanel->SetMinSize(panelSize);

        wxStaticBitmap* bmp =
            new wxStaticBitmap(iconPanel, wxID_ANY, wxBitmap(img));
        bmp->SetMinSize(panelSize);
        int x = 0;
        int y = -3;
        bmp->SetPosition(wxPoint(x, y));

        rankWidget = iconPanel;
      } else {
        wxString rankStr = wxString::Format("%d.", rank);
        wxStaticText* rankText = new wxStaticText(dlg, wxID_ANY, rankStr);
        rankText->SetFont(nameFont);
        rankText->SetMinSize(wxSize(iconSize, iconSize));
        rankWidget = MakeRankCell(rankText);
      }
      // Column 2: name
      wxString nameStr = wxString::FromUTF8(name.c_str());
      wxStaticText* nameText = new wxStaticText(dlg, wxID_ANY, nameStr);
      nameText->SetFont(nameFont);
      // Column 3: blank
      wxStaticText* statusText = new wxStaticText(dlg, wxID_ANY, "");
      statusText->SetFont(statusFont);

      grid->Add(rankWidget, 0,
                wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxRIGHT,
                5);
      grid->Add(nameText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, 0);
      grid->Add(statusText, 0,
                wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxLEFT,
                5);
    }
  }

  // Display unfinished player
  if (unfinishedPlayer != -2) {
    std::string name = getPlayerDisplayName(unfinishedPlayer);
    if (name.empty()) {
      name = "Player";
    }

    // Column 1: blank rank
    wxWindow* rankCell = MakeRankCell(nullptr);
    // Column 2: name
    wxString nameStr = wxString::FromUTF8(name.c_str());
    wxStaticText* nameText = new wxStaticText(dlg, wxID_ANY, nameStr);
    nameText->SetFont(nameFont);
    // Column 3: status
    wxString statusStr = "";
    if (!finishedPlayers.empty()) {
      statusStr = "Lost";  // only if someone finished
    }
    wxStaticText* statusText = new wxStaticText(dlg, wxID_ANY, statusStr);
    statusText->SetFont(statusFont);

    grid->Add(rankCell, 0,
              wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxRIGHT, 5);
    grid->Add(nameText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxRIGHT, 5);
    grid->Add(statusText, 0,
              wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxLEFT, 5);

    // Display a line if there are disconnected players
    if (!disconnectedPlayers.empty()) {
      wxStaticLine* line1 = new wxStaticLine(dlg, wxID_ANY);
      wxStaticLine* line2 = new wxStaticLine(dlg, wxID_ANY);
      wxStaticLine* line3 = new wxStaticLine(dlg, wxID_ANY);
      grid->Add(line1, 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
      grid->Add(line2, 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
      grid->Add(line3, 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
    }
  }

  // Display disconnected players
  if (!disconnectedPlayers.empty()) {
    for (int dp : disconnectedPlayers) {
      std::string name = getPlayerDisplayName(dp);
      if (name.empty()) {
        name = "Player";
      }

      // Column 1: blank rank
      wxWindow* rankCell = MakeRankCell(nullptr);
      // Column 2: name
      wxString nameStr = wxString::FromUTF8(name.c_str());
      wxStaticText* nameText = new wxStaticText(dlg, wxID_ANY, nameStr);
      nameText->SetFont(nameFont);
      // Column 3: status
      wxString statusStr = "Disconnected";
      wxStaticText* statusText = new wxStaticText(dlg, wxID_ANY, statusStr);
      statusText->SetFont(statusFont);

      grid->Add(rankCell, 0,
                wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxRIGHT,
                5);
      grid->Add(nameText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxRIGHT,
                5);
      grid->Add(statusText, 0,
                wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxLEFT,
                5);
    }
  }

  // Put the grid into the listSizer
  listSizer->Add(grid, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 20);

  // Add the list to the main sizer and let it expand
  mainSizer->Add(listSizer, 1, wxEXPAND);

  // Leave button at the bottom
  wxButton* closeBtn = new wxButton(dlg, wxID_ANY, "Leave");
  closeBtn->Bind(wxEVT_BUTTON, [this, dlg](wxCommandEvent&) {
    dlg->EndModal(wxID_OK);  // closes the modal dialog
    this->Close(true);       // closes MainGameFrame
  });
  mainSizer->Add(closeBtn, 0, wxALIGN_CENTER | wxALL, 15);
  dlg->SetSizer(mainSizer);
  mainSizer->SetSizeHints(dlg);  // min size based on sizer layout
  dlg->CentreOnParent();
  dlg->ShowModal();
  dlg->Destroy();
}
