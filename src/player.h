#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "changeset.h"
#include "strategy.h"

class Player : public Targetable {
public:
    Strategy& strategy;
};

#endif
