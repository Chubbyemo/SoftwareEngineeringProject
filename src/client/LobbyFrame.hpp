#ifndef LOBBYFRAME_HPP
#define LOBBYFRAME_HPP

#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "MainGamePanel.hpp"
#include "client.hpp"
/**
 * @class LobbyFrame
 * @brief Frame for the game lobby.
 * This frame displays the list of players and allows starting the game.
 */
class LobbyFrame : public wxFrame {
 public:
  /**
   * @brief Constructs the lobby frame.
   * @param parent The parent window for this frame.
   * @param client The client instance that represents the user.
   */
  LobbyFrame(wxWindow* parent, Client* client);

 private:
  Client* client;         ///< Client handling network communication.
  wxListBox* playerList;  ///< List of Players that are connected to the server
  ///< instance.
  wxButton* startGameButton;  ///< The button for starting the game.

  /**
   * @brief The function that is called when the "Ready" button is pressed.
   * @param event The specific event that triggered the function call.
   */
  void OnReadyButtonClicked(wxCommandEvent& event);

  /**
   * @brief The function that is called when the "Start Game" button is pressed.
   * @param event The specific event that triggered the function call.
   */
  void OnStartGameButtonClicked(wxCommandEvent& event);

  // /**
  //  * @brief The function that is called when the "Set AI Player" button is
  //  * pressed.
  //  * @param event The specific event that triggered the function call.
  //  */
  // void OnSetAIButtonClicked(wxCommandEvent& event);

  /**
   * @brief Callback function that is triggered when the server sends an update
   * (like update the player list).
   * @param event The specific event that triggered the function call.
   */
  void OnServerUpdate(wxThreadEvent& event);
};

#endif  // LOBBYFRAME_HPP
