# Poker Texas Hold'em version

This is a simple Texas Hold'em poker game where you play against an AI opponent .

## How to Lunch the game
To lunch the game you need to run ./poker file that is located in the build directory. Ensure that all required libraries are installed.
to rebuild the game run ./startGame.sh this will rebuild all required things. If further actions are needed, you can always modify the startGame file 

the game uses 

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

May the cards be with you, Good luck!
