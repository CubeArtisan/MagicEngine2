#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "changeset.h"
#include "strategy.h"

struct Player : public Targetable {
public:
	const std::shared_ptr<Strategy> strategy;

    Player(std::shared_ptr<Strategy> strategy)
        : strategy(strategy)
    {}
};

#endif
