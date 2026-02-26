#include "LobbyFrame.hpp"

#include <queue>
#include <string>

#include "shared/messages.hpp"

// Lobby Frame
LobbyFrame::LobbyFrame(wxWindow* parent, Client* client)
    : wxFrame(nullptr, wxID_ANY, "Game Lobby", wxDefaultPosition,
              wxSize(700, 300)),
      client(client) {
  this->SetBackgroundColour(wxColour(203, 163, 110));
  auto mainSizer = new wxBoxSizer(wxVERTICAL);

  playerList = new wxListBox(this, wxID_ANY);
  playerList->SetBackgroundColour(wxColour(240, 212, 175));
  mainSizer->Add(playerList, 1, wxALL | wxEXPAND, 10);

  // // Computer Player Count Controls
  // aiCountField = new wxSpinCtrl(
  //     this, 1, "Number of Computer Players", wxDefaultPosition, wxSize(150,
  //     -1), wxSP_ARROW_KEYS, 0, 6, 0);  // Change to SpinCtrl
  // auto setAIButton = new wxButton(this, wxID_ANY, "Set Computer Players");
  // setAIButton->Bind(wxEVT_BUTTON, &LobbyFrame::OnSetAIButtonClicked, this);

  // auto aiSizer = new wxBoxSizer(wxHORIZONTAL);
  // aiSizer->Add(aiCountField, 1, wxALL | wxEXPAND, 5);
  // aiSizer->Add(setAIButton, 0, wxALL, 5);
  // mainSizer->Add(aiSizer, 0, wxALL | wxEXPAND, 0);

  // Ready Button
  auto readyButton = new wxButton(this, wxID_ANY, "Ready");
  readyButton->Bind(wxEVT_BUTTON, &LobbyFrame::OnReadyButtonClicked, this);
  mainSizer->Add(readyButton, 0, wxALIGN_CENTER | wxALL, 5);

  // Start Game Button (Initially Disabled)
  startGameButton = new wxButton(this, wxID_ANY, "Start Game");
  startGameButton->Bind(wxEVT_BUTTON, &LobbyFrame::OnStartGameButtonClicked,
                        this);
  startGameButton->Enable(false);
  mainSizer->Add(startGameButton, 0, wxALIGN_CENTER | wxALL, 5);

  // Bind the GUI to the client's update callback
  client->setUpdateCallback([this](const std::string& message) {
    wxThreadEvent* evt = new wxThreadEvent(wxEVT_THREAD, wxID_ANY);
    evt->SetString(message);
    wxQueueEvent(this, evt);
  });

  this->Bind(wxEVT_THREAD, &LobbyFrame::OnServerUpdate, this);
  this->SetSizer(mainSizer);
}

void LobbyFrame::OnReadyButtonClicked(wxCommandEvent& event) {
  client->sendReady();
}

void LobbyFrame::OnStartGameButtonClicked(wxCommandEvent& event) {
  client->sendStartGame();
}

void LobbyFrame::OnServerUpdate(wxThreadEvent& event) {
  // Process high-level directives passed by `Client`
  nlohmann::json messageJson =
      nlohmann::json::parse(event.GetString().ToStdString());
  std::cout << event.GetString().ToStdString() << "\n";

  auto message = Message::fromJson(messageJson);
  MessageType messageType = message->getMessageType();

  switch (messageType) {
    case MessageType::BRDC_PLAYER_LIST: {
      playerList->Clear();

      std::array<PlayerStatus, 4> updatedPlayerList = client->getPlayerList();

      // Update player ID
      for (auto player : updatedPlayerList) {
        if (player.name == client->getPlayerName()) {
          client->setPlayerIndex(player.id);
        }
      }

      int localIdx = wxNOT_FOUND;
      int myIndexFromClient = client->getPlayerIndex();
      int idx = 0;

      for (const auto& player : updatedPlayerList) {
        playerList->AppendString(player.name +
                                 (player.isReady ? " (Ready)" : ""));

        if (idx == myIndexFromClient) {
          localIdx = idx;
        }
        ++idx;
      }

      if (localIdx != wxNOT_FOUND) {
        playerList->SetSelection(localIdx);  // highlight your own name
      } else {
        playerList->DeselectAll();
      }

      startGameButton->Enable(client->areAllPlayersReady());
      break;
    }

    case MessageType::BRDC_GAME_START: {
      auto* startMessage = static_cast<GameStartMessage*>(message.get());
      unsigned numPlayers = startMessage->numPlayers;
      std::cout << "Received BRDC_GAME_START with " << numPlayers << " players"
                << std::endl
                << std::flush;

      // Create MainGameFrame (but keep it hidden)
      auto mainGameFrame =
          new MainGameFrame(L"BRÄNDI DOG Multiplayer Game", client, numPlayers);
      wxPoint currentPos = this->GetPosition();
      mainGameFrame->SetPosition(currentPos);

      // Set up callback
      client->setUpdateCallback([mainGameFrame](const std::string& msg) {
        auto evt = new wxThreadEvent(wxEVT_THREAD);
        evt->SetString(msg);
        wxQueueEvent(mainGameFrame, evt);
      });

      // Process buffered messages FIRST
      client->completeTransitionToGame();

      // Process all pending events synchronously
      wxSafeYield(mainGameFrame);
      // THEN show the frame
      mainGameFrame->Show(true);

      // Clean up lobby
      this->Unbind(wxEVT_THREAD, &LobbyFrame::OnServerUpdate, this);
      this->Destroy();
      break;
    }
    // These messages shouldn't appear in the lobby context
    case MessageType::BRDC_GAMESTATE_UPDATE:
    case MessageType::BRDC_PLAYER_FINISHED:
    case MessageType::BRDC_RESULTS:
    case MessageType::BRDC_PLAYER_DISCONNECTED:  // Currently not made for lobby
                                                 // level disconnects
    case MessageType::PRIV_CARDS_DEALT:
    case MessageType::RESP_START_GAME:
    case MessageType::RESP_PLAY_CARD:
    case MessageType::RESP_SKIP_TURN:
      std::cerr << "Unexpected game message in lobby: "
                << static_cast<int>(messageType) << std::endl;
      break;
    // Client-to-server messages - should NEVER be received
    case MessageType::REQ_CONNECT:
    case MessageType::REQ_READY:
    case MessageType::REQ_START_GAME:
    case MessageType::REQ_PLAY_CARD:
    case MessageType::REQ_SKIP_TURN:
    case MessageType::RESP_CONNECT: {
      std::cerr << "Invalid client-to-server message received in lobby: "
                << static_cast<int>(messageType) << std::endl;
      break;
    }
  }
}
