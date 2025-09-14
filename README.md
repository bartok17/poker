# Poker Texas Hold'em version

This is a simple Texas Hold'em poker game where you play against an AI opponent.

## How to Launch the Game

- Quick start (recommended):
  - macOS/Linux:
    1. Make the script executable (first time only): `chmod +x ./startGame.sh`
    2. Run: `./startGame.sh`
  - Windows:
    1. Run: `startGame.bat`

- From Visual Studio Code:
  - macOS/Linux: open the integrated terminal and run `./startGame.sh`
  - Windows: open the integrated terminal (PowerShell or cmd) and run `startGame.bat`

- Build manually with CMake:
  - macOS/Linux:
    - `rm -rf build && cmake -S . -B build`
    - `cmake --build build`
    - `./build/poker`
  - Windows (PowerShell or cmd):
    - `rd /s /q build` (or delete the build folder manually)
    - `cmake -S . -B build`
    - `cmake --build build --config Release`
    - `.\build\Release\poker.exe` (or `.\build\poker.exe` for single-config generators)

Entry point: [src/main.cpp](src/main.cpp)

## Dependencies (SFML handled automatically)

- CMake first tries to find system SFML; if not found, it auto-fetches and builds SFML 2.6.1 (see [CMakeLists.txt](CMakeLists.txt)). First build may take longer.
- Windows: required SFML DLLs are copied next to the executable after build.

Optional: use a system SFML
- Default is to try system SFML first: `-DUSE_SYSTEM_SFML=ON` (then falls back to fetch if not found).
- Windows with vcpkg:
  - `vcpkg install sfml:x64-windows`
  - Configure with: `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg.cmake`

## Fonts

The game loads Arial/DejaVu automatically. If text is missing, ensure one of:
- `arial.ttf` in the project root, or system fonts:
  - Windows: `C:/Windows/Fonts/arial.ttf`
  - Linux: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
  - macOS: `/System/Library/Fonts/Supplemental/Arial.ttf`

## How to Play

### Objective
The main goal is to win all of your opponent's chips. The game ends when one player runs out of chips. Each player starts with 2000 chips.

### Game Rounds
Each round of poker consists of several phases:

1.  **Blinds:** At the beginning of each new round, both players automatically contribute a "blind" bet (default 50 chips each) to the pot to start the action.
2.  **Dealing:**
    *   Each player receives two private cards (hole cards). Your cards are always visible at the bottom of the screen.
    *   Five community cards are dealt face down in the center of the table. These will be revealed in stages.
3.  **Betting Rounds:** There are up to four rounds of betting:
    *   **Pre-Flop:** After the hole cards are dealt, the first betting round begins.
    *   **Flop:** If the game continues, three community cards are revealed. Another betting round occurs.
    *   **Turn:** A fourth community card is revealed, followed by another betting round.
    *   **River:** The fifth and final community card is revealed, followed by the last betting round.

    During a betting round, it will be your turn to act. You have several options, controlled by the buttons at the bottom of the screen:

    *   **To Set Your Bet Amount:**
        *   `+10`, `+50`, `+100`: Click these buttons to increase Your Bet (the amount you intend to bet or raise).
        *   `Reset Bet`: Clears Your Bet back to 0.

    *   **To Make Your Action:**
        *   **`Submit` (Bet/Raise):**
            *   After setting Your Bet using the buttons, click `Submit`.
            *   If the AI has not bet yet in this round, or if bets are equal, this is a **Bet**.
            *   If the AI has bet, and "Your Bet" is higher, this is a **Raise**.
            *   If the AI has bet, and "Your Bet" is equal to the AI's bet, this is a **Call**.
            *   The game will ensure your bet is at least a call if the AI has an outstanding bet (unless you are going all-in with fewer chips).
            *   If you bet all your remaining chips, you are **All-In**.
        *   **`Wait` (Check/Call):**
            *   If the AI has made a bet that you haven't matched yet (i.e., the AI's current bet for the round is higher than yours), clicking `Wait` means you **Call** the AI's bet, matching it.
            *   If bets are currently equal (e.g., at the start of a betting round after blinds, or if both players have matched previous bets), clicking `Wait` means you **Check**, passing the action to the AI without betting.
        *   **`Pass` (Fold):**
            *   Click this to forfeit the current hand. You lose any chips you've already put into the pot for this round. The AI wins the pot.

4.  **AI's Turn:** After you act, the AI will make its move (fold, check, call, bet, or raise). The game will display the AI's action. If the AI raises, you will need to act again.

5.  **Advancing the Round:**
    *   When betting is complete for a phase (e.g., both players check, or one player bets and the other calls), the next community cards are revealed (Flop, then Turn, then River).
    *   If a player goes All-In and is called, any remaining community cards are revealed immediately.

6.  **Showdown:**
    *   If betting is complete after the River card, or if an All-In player is called, a showdown occurs.
    *   Both your hand and the AI's hand are revealed (if not already).
    *   The player with the best 5-card poker hand (using any combination of their two private cards and the five community cards) wins the pot.
    *   The winner will be announced.

7.  **Next Round:**
    *   If the game is not over (neither player has run out of chips), click the `Next Round` button to start a new hand.

### Winning the Game
*   You win the game if the AI's score reaches 0.
*   You lose the game if your score reaches 0.
*   When the game is over, a message will be displayed. You can then choose to `Play Again` (resets scores to 2000) or `Quit Game`.

May the cards be with you. Good luck!
