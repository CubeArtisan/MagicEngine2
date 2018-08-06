#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "changeset.h"
#include "strategy.h"

struct Player : public clone_inherit<Player, Targetable> {
public:
	const std::shared_ptr<Strategy> strategy;

    Player(std::shared_ptr<Strategy> strategy)
        : strategy(strategy)
    {}
};

#endif
