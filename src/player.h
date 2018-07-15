#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "changeset.h"
#include "strategy.h"

class Player : public Targetable {
public:
    std::shared_ptr<Strategy> strategy;

    Player(std::shared_ptr<Strategy> strategy)
        : strategy(strategy)
    {}
};

#endif
