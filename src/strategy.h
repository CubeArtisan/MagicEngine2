#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

struct Player;
struct Environment;

class Strategy {
public:
    virtual GameAction chooseGameAction(Player& player, Environment& env) = 0;
};

class RandomStrategy {
public:
    GameAction chooseGameAction(Player& player, Environment& env);
};

#endif
