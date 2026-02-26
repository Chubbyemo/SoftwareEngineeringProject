# Brändi Dog 

*Where marbles race, cards rule, and friendships are temporarily ruined!*

![Brändi-Dog-logo](./assets/braendi_dog_logo.png)

[![Tests](https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds/-/raw/site/testing-badge.svg)](https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds/-/pipelines)
[![Coverage](https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds/-/raw/site/coverage-badge.svg)](https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds/-/pipelines)

This project is a C++ implementation of the swiss game Brändi Dog. It is designed for 2-4 players.


---

## Project Structure

* [`assets`](assets): Contains image files for the graphical user interface (GUI) and the documentation website.
<!-- * [`help`](help): Additional documentation and guides. Includes a quick checklist and suggested improvements to better understand the project structure. -->
* [`src`](src): Source code of the Brändi Dog game (.cpp and .hpp files).
* [`tests`](tests): Unit tests for the core game logic.
* [`diagrams`](diagrams): UML diagrams that visualize how the classes interact.
* **Root files**:

  * [`CMakeLists.txt`](CMakeLists.txt): Build configuration file.
  * [`.gitlab-ci.yml`](.gitlab-ci.yml): GitLab CI pipeline config. It builds the project, runs tests, generates badges (see top), and updates the documentation on GitLab Pages.
  * [`Doxyfile`](Doxyfile): Configuration file for Doxygen documentation.
  * [`.gitignore`](.gitignore): Lists files and folders to exclude from version control.

---

## Game Overview

Players move their marbles around the cross-shaped board according to the cards they play. The game combines tactical decision-making with the luck of the draw, creating a balanced and engaging experience.

### Key Features:

* **2-4 Players**: Simple to learn but with deep strategic elements
* **Card-Driven Movement**: Use cards like Aces, Kings, Jacks and number cards to navigate the board
* **Special Actions**: Exchange marbles with opponents, send opponents back home, or use special walking cards

### Game Rules

* Each player starts with 4 marbles in their home
* Players use cards to move their marbles around the board
* To get a marble out of the home, you need an **ACE**, **KING**, or **JOKER**
* The first marble placed on a start position creates a blockade that cannot be passed, even by ones own marbles
* If a marble's move lands on the same space an opponent already occupies, the opponent is sent back to it's home
* Use the **SEVEN** card to split moves between multiple marbles
* Use the **JACK** card to exchange positions with another player's marble
* Players must use all available cards in their hand each round
* If a player has no valid moves, their hand is automatically cleared and they wait for others to complete the round until new cards are distributed
* The dice on the right indicates how many cards are distributed in each round (6 cards in 1st round, 5 in 2nd, and so on until 2 cards in 5th round, then starts again with 6)
* Marbles in the finish fields cannot be jumped over - you need to use your cards to move the marbles within the finish area and fill up the fields
* The first player to get all 4 marbles to the finish wins!
* **Special Rule**: For a more exciting gameplay experience, we allow players to go backwards to reach the finish! Use your **FOUR** wisely! 

### Card Values

* **ACE**: Start or move 1 or 11 spaces
* **KING**: Start or move 13 spaces
* **QUEEN**: Move 12 spaces
* **JACK**: Exchange one of your marbles with an opponent's marble, if your marble is blocked on start you cannot swap it away
* **Number cards 2-10**: Move the corresponding number of spaces (Special: 4 or 7)
* **FOUR**: Can move 4 forward OR backward
* **SEVEN**: Can be split however many times between multiple marbles for a sum of 7 fields walked forward (no less, no more)
* **JOKER**: Use as any other card value and function

For a complete overview, refer to the official [Brändi Dog rulebook](https://www.connexxion24.com/downloads/anleitungen-braendi-dog/Spielregeln-braendi-dog-englisch.pdf).
All rules described above are implemented, they should be viewed as reference even if the official rules my slightly diverge.


---

## Key Technologies

* **wxWidgets** – GUI
* **sockpp** – TCP networking
* **nlohmann/json** – JSON serialization
* **googletest** – Unit testing

---

## Installation & Setup

We recommend **Ubuntu** as the primary platform, but the project also works on **Windows 11** and **macOS (Intel/Apple Silicon)**. If you encounter any warnings  CMake, you can ignore them for now.

---

### Ubuntu

Tested on **Ubuntu 24.04**, but other versions should also work.

#### Option 1: Using Build Scripts (Recommended)

We provide automated build and run scripts for convenience.

**Setup:**

```sh
cd <your/local/folder>
git clone https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds.git
cd the-hounds
chmod +x build.sh run.sh
```

**Build the project:**

```sh
./build.sh
```

This script will automatically:
- Install all system dependencies (clang-format, cmake, git, gnupg, g++, clang, libgtk-3-dev)
- Install documentation tools (doxygen, graphviz)
- Configure and build the project using CMake

**Run the game:**

```sh
./run.sh
```

By default, this starts one server and two clients. You can customize:

```sh
./run.sh -n 4
```

Press `Ctrl+C` to stop all processes.

#### Option 2: Manual Setup

**Install system dependencies:**

```sh
sudo apt update
sudo apt install -y clang-format cmake git gnupg g++ clang libgtk-3-dev
```

**To generate documentation, additionally install:**

```sh
sudo apt install doxygen graphviz
```

**Clone the repository:**

```sh
cd <your/local/folder>
git clone https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds.git
```

**First-Time Compilation:**

We use CPM (CMake Package Manager), which downloads and builds dependencies automatically. First-time builds may take longer.

```sh
mkdir build
cd build
cmake ..
make -j$(nproc)
```

**Start the server:**

```sh
./Server 127.0.0.1 12345
```

OR simply (for the above defaults):

```sh
./Server
```

**Run a client (in a second terminal):**

```sh
./Client
```

You can run multiple clients locally or connect over a local network.

---

### Windows

We strongly recommend using **Windows Subsystem for Linux (WSL)**.
To check your WSL version, run in PowerShell:

```sh
wsl --version
```

If not installed, follow the [official WSL installation guide](https://learn.microsoft.com/en-us/windows/wsl/install).
Use the default Ubuntu distribution.

> **Why WSL version matters:**
>
> * **WSL 1**: Fast access to Windows files (e.g., `C:\Users\...`), slower for Linux files
> * **WSL 2**: Faster within Linux filesystem, slower for Windows files
>
> For faster builds, store the repo in the file system best suited to your WSL version.

**Recommended clone locations:**

* **WSL 1**: Clone to `C:\Users\<You>\Documents\The-Hounds`
* **WSL 2**: Clone to `~/the-hounds` within the Linux environment

Once cloned, start WSL, navigate to the folder, and follow the Ubuntu instructions above.

---

### macOS

On macOS we use *Homebrew's wxWidgets*. It should work on Apple Silion and Intel.

**Install Prerequisites**
```bash
xcode-select --install
brew install cmake wxwidgets clang-format doxygen graphviz
```

**Configure & build**
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

---

## Recompiling the Code

To recompile after changes:

```sh
cd build
make
```

---

<!-- ## Using CLion

[CLion](https://www.jetbrains.com/clion/) is a full-featured C++ IDE. ETH students can access it for free.
Follow the setup guide [here](https://readme.phys.ethz.ch/documentation/jetbrains_edu_account/).

CLion offers many quality-of-life features for C++ development. We recommend getting familiar with it, though depending on your OS, another editor may be better suited.

> **Note for Windows Users**:
> CLion setup can be tricky under Windows. If configuration fails, you may find it easier to build and run the game via a WSL terminal.

**CLion Setup Steps:**

* Open CLion
* `File > Open...` → Select the project folder
* `Build > Build All in 'Debug'`
* Wait for compilation of `Server`, `Client`, and `test_game` executables

--- -->

## Running the Game & Tests

Once compiled, the following executables will appear in the `build/` directory:

* Server: `./Server <address> <port>`
* Client: `./Client`
* Tests: `./test_game`

> Use `./Server 127.0.0.1 12345` as a default value. Other values may also work depending on your system/network.
---
Alternatively, you can run the bash script `start.sh`, which will start the server and two clients.

<!-- ## Code Formatting

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and use `clang-format`.

To format the code:

```sh
clang-format -style=google -i $(find src tests -name "*.cpp" -o -name "*.hpp")
```

--- -->
---

## Documentation

Clear documentation improves knowledge transfer between developers.
This project uses [Doxygen](https://doxygen.nl/) to generate both a website and a LaTeX document from inline comments.

If installed, generate documentation locally by running:

```sh
doxygen Doxyfile
```

You can then view the documentation by:

* Opening `docs/html/index.html` in a browser, or
* Installing the "Live Server" extension in VSCode and selecting “Go Live” on the file

If your pipeline runs successfully, GitLab will also serve the latest version of your documentation under [<your-GitLab-Repo-URL>/-/tree/site/docs/](https://gitlab.inf.ethz.ch/course-secse-2025/the-hounds/-/tree/site/docs). You can download it as a zip and browse it similarily as if you had generated it yourself.

---

<!-- ## Guides

See the [`help`](help) directory for:

* [How to add a Joker card](help/add-joker-card.md)
* [How to add a Reverse card](help/add-reverse-card.md)
* [Architecture Overview](help/architecture-description.md)
* [Other testing ideas](help/other-testing-ideas.md)
* [Getting started checklist](help/getting-started.md)

--- -->
<!-- 
## Dependency Management

We use [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake), a modern C++ package manager that integrates with CMake.

To add a new dependency:

```cmake
CPMAddPackage(
    NAME <LibraryName>
    GITHUB_REPOSITORY <GitHub-Repository>
    VERSION <Version>
)
``` -->
## Images
The images used in the project were sourced from the following websites:

* The dice icon and the rules icon: https://game-icons.net/
* The user icons:
https://www.drawio.com/
* The Brändi dog logo:
https://www.braendi-dog.de/
* The poker card set:
https://gitcode.com/open-source-toolkit/e7c8d.git
* The board and the marbles are created by the-hounds using WxWidgets
---
## Authors
The Brändi Dog project was developed by the **The Hounds** team:
- **Fabienne Buchser**
- **Koukou Luo**
- **Na Wang Chu Ki Arutsang**
- **Sophie Xie**

---
