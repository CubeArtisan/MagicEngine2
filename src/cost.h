#ifndef _COST_H_
#define _COST_H_

#include "changeset.h"
#include "mana.h"

struct Environment;
struct Player;
struct CardToken;
struct Token;
struct Card;

using SourceType = std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>>;
struct Cost {
    virtual bool canPay(Player& player, Environment& env, SourceType source) = 0;
    virtual Changeset payCost(Player& player, Environment& env, SourceType source) = 0;
};

struct ManaCost : public Cost {
    virtual bool canPay(Player& player, Environment& env, SourceType source);
    virtual Changeset payCost(Player& player, Environment& env, SourceType source);

    ManaCost(Mana mana);

private:
    Mana mana;
};

struct LandPlayCost : public Cost {
    virtual bool canPay(Player& player, Environment& env, SourceType source);
    virtual Changeset payCost(Player& player, Environment& env, SourceType source);
};

struct TapCost : public Cost {
    bool canPay(Player& player, Environment& env, SourceType source);
    Changeset payCost(Player& player, Environment& env, SourceType source);
private:
};

#endif
