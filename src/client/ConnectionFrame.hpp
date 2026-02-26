#ifndef CONNECTIONFRAME_HPP
#define CONNECTIONFRAME_HPP

#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "LobbyFrame.hpp"
#include "guiHelpers.hpp"

/**
 * @class BraendiDogGame
 * @brief Main application class for the BRAENDI DOG multiplayer game.
 */
class BraendiDogGame : public wxApp {
 public:
  /**
   * @brief Initializes the GUI application.
   * @return True if the initialization is successful, false otherwise.
   */
  virtual bool OnInit() override;
};

/**
 * @class ConnectionFrame
 * @brief Frame for connecting to the game server.
 * This frame allows the user to enter the server address and port.
 */
class ConnectionFrame : public wxFrame {
 public:
  /**
   * @brief Constructs the connection frame, providing input fields for server
   * address and port. Initializes the frame layout, including an image logo,
   * server address input field, port input field, and a "connect" button. The
   * frame is displayed with default dimensions, and a connection button is
   * linked to the `OnConnectButtonClicked` event handler.
   *
   * @param parent The parent window for the frame.
   *
   * @note This constructor arranges the UI elements using a vertical box sizer
   * and adjusts the layout with `SetSizerAndFit`.
   */
  explicit ConnectionFrame(wxWindow* parent);

  /**
   * @brief This function returns the server address of the InputField.
   * @return Input of the user inside the serverAddress InputField
   */
  wxString GetServerAddress() const { return serverAddressField->getValue(); }

  /**
   * @brief This function returns the server port of the InputField
   * @return Input of the user inside the portField InputField
   */
  wxString GetPort() const { return portField->getValue(); }

  /**
   * @brief This function returns the player name the user has chosen
   * @return Input of the user inside playerName InputField
   */
  wxString getPlayerName() const { return playerNameField->getValue(); }

 private:
  /**
   * @brief This function generates a randon attribute and animal as a
   * suggestion for the player name
   * @return a random wxString player Name consisting of an attribute and an
   * animal.
   */
  wxString randNameGenerator() const;

  wxPanel* panel;                  ///< Panel for GUI elements.
  InputField* serverAddressField;  ///< Input field for server address.
  InputField* portField;           ///< Input field for port number.
  InputField* playerNameField;     ///< Input field for player name.
  wxButton* connectButton;         ///< Button to initiate connection.

  /**
   * @brief Handles the click event for the "Connect" button.
   * Retrieves the server address, port, and player name from the respective
   * input fields, then attempts to establish a connection to the server. If
   * successful, it transitions to the `LobbyFrame`. If an error occurs during
   * the connection attempt, it displays an error message to the user.
   * @param event The wxCommandEvent triggered by clicking the "Connect" button.
   * @throws std::runtime_error If the connection to the server fails.
   *
   * @note The function creates a new `Client` instance with the provided
   * connection details and transitions to the `LobbyFrame` if the connection is
   * established.
   */
  void OnConnectButtonClicked(wxCommandEvent& event);
};

#endif  // CONNECTIONFRAME_HPP
