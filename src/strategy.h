#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include "gameAction.h"

struct Player;
struct Environment;
struct HasEffect;

class Strategy {
public:
    virtual GameAction chooseGameAction(const Player& player, const Environment& env) = 0;
	virtual std::vector<xg::Guid> chooseTargets(std::shared_ptr<const HasEffect> effect, const Player& player, const Environment& env) = 0;
};

class RandomStrategy : public Strategy {
public:
    GameAction chooseGameAction(const Player& player, const Environment& env);
	std::vector<xg::Guid> chooseTargets(std::shared_ptr<const HasEffect> effect, const Player& player, const Environment& env);
};

#endif
