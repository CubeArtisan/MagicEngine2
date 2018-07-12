#ifndef _COST_H_
#define _COST_H_

#include "changeset.h"
#include "mana.h"

class Environment;
class Player;

class Cost {
public:
    virtual bool canPay(Player& player, Environment& env) = 0;
    virtual Changeset payCost(Player& player, Environment& env) = 0;
};

class ManaCost : public Cost {
public:
    virtual bool canPay(Player& player, Environment& env);
    virtual Changeset payCost(Player& player, Environment& env);

private:
    Mana mana;
};
#endif
