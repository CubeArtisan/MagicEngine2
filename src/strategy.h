#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

class Player;
class Environment;

class Strategy {
public:
    GameAction chooseGameAction(Player& player, Environment& env);
};

#endif
