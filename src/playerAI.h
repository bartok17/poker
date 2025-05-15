#ifndef PLAYERAI_H
#define PLAYERAI_H

#include "hand.h"
#include "card.h"
#include "comparer.h"
#include "deck.h"


class playerAI
{
public:
    
    
    double evaluateHand(const Hand& hand, const std::vector<Card>& board);

    bool simulateWin(Hand myHand, std::vector<Card> board);
};

#endif // PLAYERAI_H