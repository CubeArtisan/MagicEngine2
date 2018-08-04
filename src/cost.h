#ifndef _COST_H_
#define _COST_H_

#include "changeset.h"
#include "mana.h"

struct Environment;
struct Player;
struct CardToken;
struct Token;
struct Card;
struct Emblem;

using SourceType = std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>, std::shared_ptr<const Emblem>>;
struct Cost {
    virtual bool canPay(const Player& player, const Environment& env, const SourceType& source) const = 0;
    virtual Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const = 0;
};

struct ManaCost : public Cost {
    virtual bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    virtual Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;

    ManaCost(Mana mana);

private:
    Mana mana;
};

struct LandPlayCost : public Cost {
    virtual bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    virtual Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;
};

struct TapCost : public Cost {
    bool canPay(const Player& player, const Environment& env, const SourceType& source) const;
    Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const;
private:
};

#endif
