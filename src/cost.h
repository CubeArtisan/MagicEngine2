#ifndef _COST_H_
#define _COST_H_

#include "changeset.h"
#include "mana.h"

struct Environment;
struct Player;
struct CardToken;

struct Cost {
    virtual bool canPay(Player& player, Environment& env) = 0;
    virtual Changeset payCost(Player& player, Environment& env) = 0;
};

struct ManaCost : public Cost {
    virtual bool canPay(Player& player, Environment& env);
    virtual Changeset payCost(Player& player, Environment& env);

    ManaCost(Mana mana);

private:
    Mana mana;
};

struct LandPlayCost : public Cost {
    virtual bool canPay(Player& player, Environment& env);
    virtual Changeset payCost(Player& player, Environment& env);
};

struct TapCost : public Cost {
    bool canPay(Player& player, Environment& env);
    Changeset payCost(Player& player, Environment& env);

    TapCost(CardToken& object);
private:
    CardToken& object;
};

#endif
