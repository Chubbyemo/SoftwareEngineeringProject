# Message Specification for Brändi Dog

**Version:** Updated to match actual implementation (December 2024)

This document describes the actual message protocol implemented in `messages.hpp` and `messages.cpp`.

## Message Categories

- **REQ_***: Client-to-Server requests
- **RESP_***: Server-to-Client responses
- **BRDC_***: Server-to-All-Clients broadcasts
- **PRIV_***: Server-to-Specific-Client private messages

---

## Message Type Enumeration

The following message types are implemented (in order):

```cpp
enum class MessageType {
  // Server-to-Client responses (0-4)
  RESP_CONNECT,
  RESP_READY,
  RESP_START_GAME,
  RESP_PLAY_CARD,
  RESP_SKIP_TURN,
  
  // Server broadcast messages (6-11)
  BRDC_PLAYER_LIST,
  BRDC_GAME_START,
  BRDC_GAMESTATE_UPDATE,
  BRDC_PLAYER_DISCONNECTED,
  BRDC_PLAYER_FINISHED,
  BRDC_RESULTS,
  
  // Server private messages (12)
  PRIV_CARDS_DEALT,
  
  // Client-to-Server requests (13-17)
  REQ_CONNECT,
  REQ_READY,
  REQ_START_GAME,
  REQ_PLAY_CARD,
  REQ_SKIP_TURN
};
```

---

## Client-to-Server Requests

### 1. REQ_CONNECT
**Direction:** Client → Server  
**Purpose:** Establishes connection with server and registers player  
**Trigger:** User enters username and clicks "Connect" in ConnectionFrame

**JSON Structure:**
```json
{
  "msgType": "REQ_CONNECT",
  "name": "string"
}
```

**Fields:**
- `name` (string): Player's chosen display name

**Server Processing:**
1. Check if server has available slot (max 4 players)
2. Validate username is non-empty and unique
3. If valid: assign next available playerID (0-3)
4. Store player info and socket mapping
5. Send RESP_CONNECT to requesting client
6. Send BRDC_PLAYER_LIST to all clients

**Expected Response:** RESP_CONNECT  
**Followed By:** BRDC_PLAYER_LIST (broadcast to all)

**Implementation Class:** `ConnectionRequestMessage`

---

### 2. REQ_READY
**Direction:** Client → Server  
**Purpose:** Player signals they are ready to start the game  
**Trigger:** User clicks "Ready" button in LobbyFrame

**JSON Structure:**
```json
{
  "msgType": "REQ_READY",
  "playerId": 0
}
```

**Fields:**
- `playerId` (size_t): ID of the player marking themselves ready

**Server Processing:**
1. Validate playerId is valid and connected
2. Set player's ready status to true
3. Send RESP_READY to requesting client
4. Send BRDC_PLAYER_LIST to all clients (includes updated ready status)
5. If all players ready: enable "Start Game" button for host

**Expected Response:** RESP_READY  
**Followed By:** BRDC_PLAYER_LIST (broadcast to all)

**Implementation Class:** `ReadyMessage`

---

### 3. REQ_START_GAME
**Direction:** Client → Server  
**Purpose:** Requests to start the game (host only)  
**Trigger:** Player 0 (host) clicks "Start Game" button in LobbyFrame

**JSON Structure:**
```json
{
  "msgType": "REQ_START_GAME",
  "playerId": 0
}
```

**Fields:**
- `playerId` (size_t): ID of requesting player (must be 0 - the host)

**Server Processing:**
1. Validate requesting player is player 0 (host)
2. Check minimum player count (at least 2 players)
3. Check all players are ready
4. Initialize GameState with connected players
5. Send RESP_START_GAME to requesting client
6. Send BRDC_GAME_START to all clients
7. Deal initial cards and send PRIV_CARDS_DEALT to each player

**Expected Response:** RESP_START_GAME  
**Followed By:**
- BRDC_GAME_START (to all clients)
- PRIV_CARDS_DEALT (to each player individually)

**Implementation Class:** `StartGameRequestMessage`

---

### 4. REQ_PLAY_CARD
**Direction:** Client → Server  
**Purpose:** Requests to play a card and execute a move  
**Trigger:** Player selects card, marble, and target position via MovePhaseController

**JSON Structure:**
```json
{
  "msgType": "REQ_PLAY_CARD",
  "playerId": 0,
  "move": {
    "cardId": 42,
    "handIndex": 2,
    "movements": [
      {
        "marbleId": {
          "playerId": 0,
          "marbleIndex": 1
        },
        "targetPos": {
          "boardLocation": "TRACK",
          "index": 15,
          "playerId": 0
        }
      }
    ]
  }
}
```

**Fields:**
- `playerId` (size_t): ID of player making the move
- `move` (Move object): Contains:
  - `cardId` (size_t): Index of card in GameState.deck being played
  - `handIndex` (size_t): Index of card in player's hand
  - `movements` (array): Array of marble movements, each containing:
    - `marbleId`: Object with `playerId` and `marbleIndex`
    - `targetPos`: Object with `boardLocation` (HOME/TRACK/FINISH), `index`, `playerId`

**Server Processing:**
1. Verify it's the requesting player's turn
2. Verify player has the specified card in hand
3. Call `GameState::isValidTurn(move)` to validate move legality
4. If valid: call `GameState::executeMove(move)`
5. Check if player finished: update leaderboard if needed
6. Call `GameState::endTurn()` to update game state
7. Check if round ended: deal new cards if needed
8. Check if game ended: send results if needed
9. Send RESP_PLAY_CARD to requesting client
10. Send BRDC_GAMESTATE_UPDATE to all clients

**Expected Response:** RESP_PLAY_CARD  
**Followed By:**
- BRDC_GAMESTATE_UPDATE (always)
- BRDC_PLAYER_FINISHED (if player finished)
- PRIV_CARDS_DEALT to each player (if round ended)
- BRDC_RESULTS (if game ended)

**Implementation Class:** `PlayCardRequestMessage`

**Note:** The Move object is defined in `game_types.hpp` and includes nested Position and MarbleIdentifier structures.

---

### 5. REQ_SKIP_TURN
**Direction:** Client → Server  
**Purpose:** Player folds/skips turn when no valid moves are available  
**Trigger:** MovePhaseController determines no legal moves exist

**JSON Structure:**
```json
{
  "msgType": "REQ_SKIP_TURN",
  "playerId": 0
}
```

**Fields:**
- `playerId` (size_t): ID of the player skipping their turn

**Server Processing:**
1. Verify it's the requesting player's turn
2. Verify player has no valid moves (security check via `GameState::hasLegalMoves()`)
3. Call `GameState::executeFold()` to mark player inactive for round
4. Call `GameState::endTurn()` to update current player
5. Check if round ended: deal new cards if needed
6. Send RESP_SKIP_TURN to requesting client
7. Send BRDC_GAMESTATE_UPDATE to all clients

**Expected Response:** RESP_SKIP_TURN  
**Followed By:**
- BRDC_GAMESTATE_UPDATE (always)
- PRIV_CARDS_DEALT to each player (if round ended)

**Implementation Class:** `SkipTurnRequestMessage`

---

## Server-to-Client Responses

### 6. RESP_CONNECT
**Direction:** Server → Client  
**Purpose:** Acknowledges connection attempt (success or failure)  
**Trigger:** Server receives REQ_CONNECT

**JSON Structure:**
```json
{
  "msgType": "RESP_CONNECT",
  "success": true,
  "errorMsg": "",
  "playerId": 0
}
```

**Fields:**
- `success` (bool): Whether connection was successful
- `errorMsg` (string): Error description if failed (empty if success)
- `playerId` (size_t): Assigned player ID (0-3, only valid if success is true)

**Possible Error Messages:**
- "Server is full" (4 players already connected)
- "Username already taken"
- "Invalid username" (empty or whitespace only)

**Expected Response:** None  
**Followed By:** BRDC_PLAYER_LIST (if success)

**Implementation Class:** `ConnectionResponseMessage`

---

### 7. RESP_READY
**Direction:** Server → Client  
**Purpose:** Confirms ready status update  
**Trigger:** Server receives REQ_READY

**JSON Structure:**
```json
{
  "msgType": "RESP_READY",
  "success": true,
  "errorMsg": ""
}
```

**Fields:**
- `success` (bool): Whether ready update was successful
- `errorMsg` (string): Error description if failed

**Expected Response:** None  
**Followed By:** BRDC_PLAYER_LIST (if success)

**Implementation Class:** `ReadyResponseMessage`

---

### 8. RESP_START_GAME
**Direction:** Server → Client  
**Purpose:** Confirms game start attempt  
**Trigger:** Server receives REQ_START_GAME

**JSON Structure:**
```json
{
  "msgType": "RESP_START_GAME",
  "success": true,
  "errorMsg": ""
}
```

**Fields:**
- `success` (bool): Whether game start succeeded
- `errorMsg` (string): Error description if failed

**Possible Error Messages:**
- "Only host can start game" (playerId != 0)
- "Need at least 2 players to start"
- "Not all players are ready"

**Expected Response:** None  
**Followed By (if success):**
- BRDC_GAME_START
- PRIV_CARDS_DEALT to each player

**Implementation Class:** `StartGameResponseMessage`

---

### 9. RESP_PLAY_CARD
**Direction:** Server → Client  
**Purpose:** Confirms move execution (success or failure)  
**Trigger:** Server receives REQ_PLAY_CARD

**JSON Structure:**
```json
{
  "msgType": "RESP_PLAY_CARD",
  "handIndex": 2,
  "success": true,
  "errorMsg": ""
}
```

**Fields:**
- `handIndex` (size_t): Index of the card that was played (or attempted)
- `success` (bool): Whether the move was valid and executed
- `errorMsg` (string): Error description if move was invalid

**Possible Error Messages:**
- "Not your turn"
- "Invalid move"
- "Card not in hand"
- "Invalid marble position"

**Expected Response:** None  
**Followed By (if success):**
- BRDC_GAMESTATE_UPDATE

**Implementation Class:** `PlayCardResponseMessage`

---

### 10. RESP_SKIP_TURN
**Direction:** Server → Client  
**Purpose:** Confirms skip turn request  
**Trigger:** Server receives REQ_SKIP_TURN

**JSON Structure:**
```json
{
  "msgType": "RESP_SKIP_TURN",
  "success": true,
  "errorMsg": ""
}
```

**Fields:**
- `success` (bool): Whether the skip was valid and processed
- `errorMsg` (string): Error description if invalid

**Possible Error Messages:**
- "Not your turn"
- "You have legal moves available" (security check failed)

**Expected Response:** None  
**Followed By (if success):**
- BRDC_GAMESTATE_UPDATE

**Implementation Class:** `SkipTurnResponseMessage`

---

## Server Broadcast Messages

### 11. BRDC_PLAYER_LIST
**Direction:** Server → All Clients  
**Purpose:** Updates all clients with current player list and ready status  
**Trigger:** 
- After successful REQ_CONNECT
- After REQ_READY
- After player disconnect

**JSON Structure:**
```json
{
  "msgType": "BRDC_PLAYER_LIST",
  "playersList": [
    {
      "id": 0,
      "name": "Alice",
      "ready": true
    },
    {
      "id": 1,
      "name": "Bob",
      "ready": false
    }
  ]
}
```

**Fields:**
- `playersList` (array of PlayerInfo): Array of connected players, each containing:
  - `id` (size_t): Player ID (0-3)
  - `name` (string): Player's display name
  - `ready` (bool): Whether player is ready to start

**Client Processing:**
- Update lobby display with player names
- Show ready status for each player
- Enable/disable "Start Game" button based on all ready

**Expected Response:** None

**Implementation Class:** `PlayerListUpdateMessage`

**Note:** PlayerInfo struct is defined in messages.hpp

---

### 12. BRDC_GAME_START
**Direction:** Server → All Clients  
**Purpose:** Notifies all clients that game has started  
**Trigger:** Server successfully processes REQ_START_GAME

**JSON Structure:**
```json
{
  "msgType": "BRDC_GAME_START",
  "numPlayers": 3
}
```

**Fields:**
- `numPlayers` (size_t): Number of players in the game (2-4)

**Client Processing:**
- Transition from LobbyFrame to MainGameFrame
- Initialize game board UI
- Wait for PRIV_CARDS_DEALT and BRDC_GAMESTATE_UPDATE

**Expected Response:** None  
**Followed By:** PRIV_CARDS_DEALT to each player

**Implementation Class:** `GameStartMessage`

**Note:** This differs from original spec which sent full GameState. Implementation sends only player count, with full state coming in subsequent BRDC_GAMESTATE_UPDATE.

---

### 13. BRDC_GAMESTATE_UPDATE
**Direction:** Server → All Clients  
**Purpose:** Synchronizes all clients with authoritative game state  
**Trigger:**
- After successful move execution (REQ_PLAY_CARD)
- After skip turn (REQ_SKIP_TURN)
- After player disconnect
- Start of new round

**JSON Structure:**
```json
{
  "msgType": "BRDC_GAMESTATE_UPDATE",
  "gameState": {
    "deck": [...],
    "players": [...],
    "currentPlayer": 1,
    "roundStartPlayer": 0,
    "roundCardCount": 6,
    "lastPlayedCard": 42,
    "leaderBoard": [null, null, null, null]
  }
}
```

**Fields:**
- `gameState` (GameState object): Complete game state (see GameState Serialization below)

**Client Processing:**
- Update local GameState copy
- Redraw game board with new marble positions
- Update current player indicator
- Update UI elements (cards, player status, etc.)

**Expected Response:** None

**Implementation Class:** `GameStateUpdateMessage`

---

### 14. BRDC_PLAYER_DISCONNECTED
**Direction:** Server → All Clients  
**Purpose:** Notifies that a player has disconnected  
**Trigger:**
- Connection timeout detected
- Socket error/close detected
- Player process terminates

**JSON Structure:**
```json
{
  "msgType": "BRDC_PLAYER_DISCONNECTED",
  "playerId": 2
}
```

**Fields:**
- `playerId` (size_t): ID of the disconnected player

**Server Processing:**
1. Mark player as inactive in GameState
2. If game in progress: call `GameState::disconnectPlayer(playerId)`
3. Update leaderboard if needed
4. Send BRDC_PLAYER_DISCONNECTED to remaining clients
5. Send BRDC_GAMESTATE_UPDATE with updated state
6. Check if game can continue (need at least 1 other player)

**Client Processing:**
- Update UI to show player as disconnected
- If in lobby: update player list
- If in game: update game board

**Expected Response:** None  
**Followed By:** BRDC_GAMESTATE_UPDATE (if game continues)

**Implementation Class:** `PlayerDisconnectedMessage`

**Note:** Currently no REQ_DISCONNECT - all disconnects are detected via connection loss.

---

### 15. BRDC_PLAYER_FINISHED
**Direction:** Server → All Clients  
**Purpose:** Announces that a player has moved all marbles to finish  
**Trigger:** GameState detects player finished after move execution

**JSON Structure:**
```json
{
  "msgType": "BRDC_PLAYER_FINISHED",
  "playerId": 1
}
```

**Fields:**
- `playerId` (size_t): ID of the player who finished

**Client Processing:**
- Update UI to show player as finished
- Play celebration animation/sound (if implemented)
- Update leaderboard display

**Expected Response:** None

**Implementation Class:** `PlayerFinishedMessage`

**Status:** ⚠️ **PARTIALLY IMPLEMENTED**  
Currently only sends playerId. Original spec called for additional fields:
- `username` (string)
- `finishPosition` (size_t)
- `elapsedTime` (size_t)

These fields are not yet implemented. TODO marker exists in code at line 675.

---

### 16. BRDC_RESULTS
**Direction:** Server → All Clients  
**Purpose:** Provides final game results and rankings  
**Trigger:** Game ends (only 0-1 players remain active)

**JSON Structure:**
```json
{
  "msgType": "BRDC_RESULTS",
  "rankings": [0, 2, 1, null]
}
```

**Fields:**
- `rankings` (array of 4 optional ints): Player IDs in finish order
  - Index 0: First place player ID (or null if no finishers)
  - Index 1: Second place player ID (or null)
  - Index 2: Third place player ID (or null)
  - Index 3: Fourth place player ID (or null)

**Client Processing:**
- Display final results screen
- Show player rankings
- Provide option to return to lobby or exit

**Server Processing:**
- Call `GameState::endTurn()` which returns game-ended flag
- Extract leaderBoard from GameState
- Send BRDC_RESULTS to all clients
- Optionally reset server state for new game

**Expected Response:** None

**Implementation Class:** `GameResultsMessage`

**Status:** ⚠️ **PARTIALLY IMPLEMENTED**  
Currently sends simplified ranking array. Original spec called for detailed leaderboard with:
- `playerId` (size_t)
- `username` (string)
- `finishPosition` (size_t)
- `finishTime` (size_t | null)
- `totalGameTime` (size_t)

These additional fields are not yet implemented. TODO marker exists in code at line 689.

---

## Server Private Messages

### 17. PRIV_CARDS_DEALT
**Direction:** Server → Specific Client  
**Purpose:** Privately sends cards dealt to a player  
**Trigger:**
- After BRDC_GAME_START (initial deal)
- After round ends (new round deal)

**JSON Structure:**
```json
{
  "msgType": "PRIV_CARDS_DEALT",
  "playerId": 0,
  "cards": [5, 12, 23, 37, 41, 50]
}
```

**Fields:**
- `playerId` (size_t): ID of the recipient player
- `cards` (array of size_t): Indices of cards in GameState.deck dealt to this player

**Server Processing:**
1. Call `GameState::dealCards()` which returns map of playerID → card indices
2. Send PRIV_CARDS_DEALT to each player with their specific cards
3. Only the recipient can see their own cards (privacy maintained)

**Client Processing:**
- Update local hand with dealt card indices
- Display cards in UI
- Enable card selection for player's turn

**Expected Response:** None

**Implementation Class:** `CardsDealtMessage`

**Security Note:** Each player receives only their own cards. Cards are identified by index in the master deck array, not by rank/suit directly.

---

## GameState Serialization Structure

The `gameState` object that appears in BRDC_GAMESTATE_UPDATE is serialized from the `BraendiDog::GameState` class:

### Complete GameState JSON
```json
{
  "deck": [
    {
      "rank": 0,
      "suit": 0
    },
    ...
  ],
  "players": [
    {
      "id": 0,
      "name": "Alice",
      "hand": [5, 12, 23, 37, 41, 50],
      "marbles": [
        {
          "boardLocation": "HOME",
          "index": 0,
          "playerId": 0
        },
        {
          "boardLocation": "TRACK",
          "index": 42,
          "playerId": 0
        },
        {
          "boardLocation": "FINISH",
          "index": 3,
          "playerId": 0
        },
        {
          "boardLocation": "HOME",
          "index": 1,
          "playerId": 0
        }
      ],
      "startField": 0,
      "activeInGame": true,
      "activeInRound": true
    },
    null,
    {
      "id": 2,
      "name": "Bob",
      ...
    },
    null
  ],
  "currentPlayer": 0,
  "roundStartPlayer": 0,
  "roundCardCount": 6,
  "lastPlayedCard": 42,
  "leaderBoard": [null, null, null, null]
}
```

### Field Descriptions

**Top-level GameState fields:**
- `deck` (array of 54 Card objects): Complete deck
  - `rank` (int): 0-12 for ACE-KING, 13 for JOKER
  - `suit` (int): 0-3 for CLUBS/DIAMONDS/HEARTS/SPADES, 4 for JOKER
- `players` (array of 4 optional Player objects): Player slots (null if empty)
- `currentPlayer` (size_t): Index (0-3) of player whose turn it is
- `roundStartPlayer` (size_t): Index of player who started current round
- `roundCardCount` (size_t): Number of cards dealt this round (6→5→4→3→2→repeat)
- `lastPlayedCard` (optional size_t): Index in deck of last played card (null at round start)
- `leaderBoard` (array of 4 optional ints): Player IDs in finish order (null for unfinished)

**Player object fields:**
- `id` (size_t): Player ID (0-3)
- `name` (string): Display name
- `hand` (array of size_t): Indices of cards in deck that player holds
- `marbles` (array of 4 Position objects): Positions of player's 4 marbles
- `startField` (size_t): This player's start field position on track (0, 16, 32, or 48)
- `activeInGame` (bool): Player is still in game (not disconnected/eliminated)
- `activeInRound` (bool): Player can still play this round (hasn't folded)

**Position object fields:**
- `boardLocation` (string): "HOME", "TRACK", or "FINISH"
- `index` (size_t): 
  - HOME: 0-3 (home slot)
  - TRACK: 0-63 (position on main track)
  - FINISH: 0-3 (finish slot)
- `playerId` (size_t): Owner of the board section (for HOME/FINISH)

### Important Notes

1. **Privacy:** Full GameState includes all players' hands. Clients receive this but should only display their own cards.

2. **Deck Indices:** Cards are referenced by index (0-53) in the deck array, not by rank/suit directly.

3. **Null Players:** Player array indices represent fixed slots. Null entries indicate no player in that slot.

4. **Marble Positions:** Board state is reconstructed from marble positions, not stored separately.

5. **Round Progression:** roundCardCount cycles: 6 → 5 → 4 → 3 → 2 → 6 → ...

6. **Finish Order:** leaderBoard[0] is 1st place, leaderBoard[1] is 2nd, etc. Null means unfinished.

---

## Message Flow Examples

### Example 1: Game Setup Flow
```
1. Client A → Server: REQ_CONNECT { name: "Alice" }
2. Server → Client A: RESP_CONNECT { success: true, playerId: 0 }
3. Server → All: BRDC_PLAYER_LIST { playersList: [{id:0, name:"Alice", ready:false}] }

4. Client B → Server: REQ_CONNECT { name: "Bob" }
5. Server → Client B: RESP_CONNECT { success: true, playerId: 1 }
6. Server → All: BRDC_PLAYER_LIST { playersList: [{id:0,...}, {id:1,...}] }

7. Client A → Server: REQ_READY { playerId: 0 }
8. Server → Client A: RESP_READY { success: true }
9. Server → All: BRDC_PLAYER_LIST { playersList: [{id:0, ready:true}, {id:1, ready:false}] }

10. Client B → Server: REQ_READY { playerId: 1 }
11. Server → Client B: RESP_READY { success: true }
12. Server → All: BRDC_PLAYER_LIST { playersList: [{id:0, ready:true}, {id:1, ready:true}] }

13. Client A → Server: REQ_START_GAME { playerId: 0 }
14. Server → Client A: RESP_START_GAME { success: true }
15. Server → All: BRDC_GAME_START { numPlayers: 2 }
16. Server → Client A: PRIV_CARDS_DEALT { playerId: 0, cards: [...] }
17. Server → Client B: PRIV_CARDS_DEALT { playerId: 1, cards: [...] }
18. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
```

### Example 2: Play Card Flow
```
1. Client A → Server: REQ_PLAY_CARD { playerId: 0, move: {...} }
2. Server → Client A: RESP_PLAY_CARD { handIndex: 2, success: true }
3. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
```

### Example 3: Round End Flow
```
1. Client A → Server: REQ_SKIP_TURN { playerId: 0 }
   (Last active player in round)
2. Server → Client A: RESP_SKIP_TURN { success: true }
3. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
   (Round ended, new round started)
4. Server → Client A: PRIV_CARDS_DEALT { playerId: 0, cards: [...] }
5. Server → Client B: PRIV_CARDS_DEALT { playerId: 1, cards: [...] }
6. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
   (Updated with new round state)
```

### Example 4: Player Finish Flow
```
1. Client A → Server: REQ_PLAY_CARD { playerId: 0, move: {...} }
   (Move gets last marble to finish)
2. Server → Client A: RESP_PLAY_CARD { handIndex: 1, success: true }
3. Server → All: BRDC_PLAYER_FINISHED { playerId: 0 }
4. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
   (leaderBoard[0] = 0)
```

### Example 5: Game End Flow
```
1. Client B → Server: REQ_PLAY_CARD { playerId: 1, move: {...} }
   (Last active player finishes)
2. Server → Client B: RESP_PLAY_CARD { handIndex: 3, success: true }
3. Server → All: BRDC_PLAYER_FINISHED { playerId: 1 }
4. Server → All: BRDC_GAMESTATE_UPDATE { gameState: {...} }
5. Server → All: BRDC_RESULTS { rankings: [0, 1, null, null] }
   (Game over: Alice 1st, Bob 2nd)
```

---

## Implementation Notes

### Not Implemented (from original spec)

The following message types from the original specification are **NOT implemented** in the current codebase:

1. **REQ_DISCONNECT / RESP_DISCONNECT**: No explicit disconnect request. Disconnects are detected via connection timeout/socket errors.

2. **HEARTBEAT**: No heartbeat mechanism implemented. Connection health monitored via socket state.

3. **ERROR**: No generic error message type. Errors communicated via response message errorMsg fields.

### Partial Implementations

The following messages are implemented but **incomplete**:

1. **BRDC_PLAYER_FINISHED**: Missing `username`, `finishPosition`, `elapsedTime` fields

2. **BRDC_RESULTS**: Missing detailed leaderboard structure with usernames and times

### Implementation Differences

1. **BRDC_GAME_START**: Sends `numPlayers` instead of full `gameState`

2. **BRDC_PLAYER_LIST**: Includes `ready` status (not in original spec)

3. **REQ_READY / RESP_READY**: Added feature not in original spec

### Security Considerations

1. **Move Validation**: Server always validates moves even if client pre-validates
2. **Turn Verification**: Server enforces turn order
3. **Card Ownership**: Server verifies player actually has the card they're playing
4. **Skip Turn**: Server verifies player truly has no valid moves before allowing skip

### Connection Management

- No explicit disconnect protocol - disconnects detected passively
- Server maintains socket connections in `Server::players_` array
- Client threads handle message listening and disconnection detection
- Disconnected players marked inactive but remain in GameState for result tracking

---

## Appendix: Type Definitions

### Enumerations

**Rank** (from game_types.hpp):
```cpp
enum class Rank {
  ACE = 0,    TWO,     THREE,   FOUR,
  FIVE,       SIX,     SEVEN,   EIGHT,
  NINE,       TEN,     JACK,    QUEEN,
  KING,       JOKER
};
```

**Suit** (from game_types.hpp):
```cpp
enum class Suit {
  CLUBS = 0,
  DIAMONDS,
  HEARTS,
  SPADES,
  JOKER
};
```

**BoardLocation** (from game_types.hpp):
```cpp
enum class BoardLocation {
  HOME,
  TRACK,
  FINISH
};
```

**MoveType** (from game_types.hpp):
```cpp
enum class MoveType {
  START,
  SIMPLE,
  SEVEN,
  SWAP,
  JOKER
};
```

### Structures

**Position** (from game_types.hpp):
```cpp
struct Position {
  BoardLocation boardLocation;
  size_t index;
  size_t playerId;
};
```

**MarbleIdentifier** (from game_types.hpp):
```cpp
struct MarbleIdentifier {
  size_t playerId;
  size_t marbleIndex;
};
```

**Move** (from game_types.hpp):
```cpp
struct Move {
  size_t cardId;
  size_t handIndex;
  std::vector<std::pair<MarbleIdentifier, Position>> movements;
};
```

---

**Document Version:** 1.0  
**Last Updated:** December 2024  
**Implementation:** messages.hpp, messages.cpp  
**Author:** Based on actual codebase analysis
