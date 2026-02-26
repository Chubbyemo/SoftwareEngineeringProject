#include "ConnectionFrame.hpp"

wxIMPLEMENT_APP(BraendiDogGame);

bool BraendiDogGame::OnInit() {
  wxInitAllImageHandlers();

  // Create the connection frame
  auto connectionFrame = new ConnectionFrame(nullptr);
  connectionFrame->Show(true);
  return true;
}
// Connection Frame
ConnectionFrame::ConnectionFrame(wxWindow* parent)
    : wxFrame(nullptr, wxID_ANY, "Connect to Server", wxDefaultPosition,
              wxSize(400, 550)) {
  // Background panel
  this->panel = new wxPanel(this);
  this->panel->SetBackgroundColour(wxColour(203, 163, 110));

  auto verticalLayout = new wxBoxSizer(wxVERTICAL);

  auto logo =
      new ImagePanel(this->panel, "../assets/braendi_dog_logo.png",
                     wxBITMAP_TYPE_ANY, wxDefaultPosition, wxSize(200, 250));
  verticalLayout->Add(logo, 0, wxALIGN_CENTER | wxTOP | wxLEFT | wxRIGHT, 10);

  this->serverAddressField =
      new InputField(this->panel, "Server Address:", 125, "127.0.0.1", 240);
  verticalLayout->Add(this->serverAddressField, 0,
                      wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

  this->portField = new InputField(this->panel, "Port:", 125, "12345", 240);
  verticalLayout->Add(this->portField, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT,
                      10);

  this->playerNameField = new InputField(this->panel, "Player Name:", 125,
                                         randNameGenerator(), 240);
  verticalLayout->Add(this->playerNameField, 0,
                      wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

  this->connectButton = new wxButton(this->panel, wxID_ANY, "Connect",
                                     wxDefaultPosition, wxDefaultSize);
  this->connectButton->Bind(wxEVT_BUTTON,
                            &ConnectionFrame::OnConnectButtonClicked, this);
  verticalLayout->Add(this->connectButton, 0, wxALIGN_CENTER | wxALL, 10);

  this->panel->SetSizer(verticalLayout);
  auto frameSizer = new wxBoxSizer(wxVERTICAL);
  frameSizer->Add(this->panel, 1, wxEXPAND);
  this->SetSizer(frameSizer);
  this->Layout();
  this->SetClientSize(wxSize(400, 550));
  this->SetMinClientSize(wxSize(400, 550));
  this->SetMaxClientSize(wxSize(400, 550));
}

void ConnectionFrame::OnConnectButtonClicked(wxCommandEvent& event) {
  std::cout << "Connect Button Clicked\n" << std::flush;
  std::string serverAddress = serverAddressField->getValue().ToStdString();
  std::cout << "Server Address " << serverAddress << "\n" << std::flush;
  int port = std::stoi(portField->getValue().ToStdString());
  std::cout << "Port " << port << "\n" << std::flush;
  std::string playerName = playerNameField->getValue().ToStdString();
  std::cout << "Player Name " << playerName << "\n" << std::flush;

  try {
    auto client = new Client(serverAddress, port, playerName);
    // Transition to the LobbyFrame
    auto lobbyFrame = new LobbyFrame(this, client);

    // get current frame's position
    wxPoint currentPos = this->GetPosition();

    // Set the position of the new frame to be the same as the connection Frame
    lobbyFrame->SetPosition(currentPos);
    lobbyFrame->Show(true);

    this->Close();  // Close the ConnectionFrame
    std::cout << "Closing Connection Frame" << std::endl;
  } catch (const std::runtime_error& e) {
    wxMessageBox("Failed to connect to server: " + std::string(e.what()),
                 "Error", wxOK | wxICON_ERROR);
  }
}

wxString ConnectionFrame::randNameGenerator() const {
  // Generate a random name for the player
  const std::vector<std::string> names_1 = {
      "The Brave",   "The Wise",     "The Swift",    "The Bold",
      "The Cunning", "The Fearless", "The Just",     "The Kind",
      "The Strong",  "The Clever",   "The Quick",    "The Loyal",
      "The Daring",  "The Witty",    "The Charming", "The Fierce",
      "The Gentle",  "The Creative", "The Cheery",   "The Radiant"};

  const std::vector<std::string> names_2 = {
      "Lama",  "Emu",   "Penguin", "Stingray", "Tiger", "Lion",  "Cheetah",
      "Zebra", "Panda", "Koala",   "Dolphin",  "Whale", "Eagle", "Falcon",
      "Hawk",  "Owl",   "Fox",     "Wolf",     "Bear"};

  // Seed the random number generator
  srand(static_cast<unsigned int>(time(nullptr)));
  return wxString(names_1[rand() % names_1.size()] + " " +
                  names_2[rand() % names_2.size()]);
}
