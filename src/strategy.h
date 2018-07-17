#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

struct Player;
struct Environment;
struct HasEffect;

class Strategy {
public:
    virtual GameAction chooseGameAction(Player& player, Environment& env) = 0;
	virtual std::vector<xg::Guid> chooseTargets(std::shared_ptr<HasEffect> effect, Player& player, const Environment& env) = 0;
};

class RandomStrategy : public Strategy {
public:
    GameAction chooseGameAction(Player& player, Environment& env);
	std::vector<xg::Guid> chooseTargets(std::shared_ptr<HasEffect> effect, Player& player, const Environment& env);
};

#endif
