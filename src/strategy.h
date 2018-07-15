#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

class Player;
class Environment;

class Strategy {
public:
    virtual GameAction chooseGameAction(Player& player, Environment& env) = 0;
};

class RandomStrategy {
public:
    GameAction chooseGameAction(Player& player, Environment& env);
};

#endif
