#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

class Ability : public Targetable {
public:
    //CodeReview: Move onto Environment
    std::vector<std::reference_wrapper<Targetable>> targets;
    Player& controller;
    
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;
    std::set<Color> colors;

    bool is_legal(int pos, Targetable& target);
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>);

    Changeset apply_effect(const Environment& env);
};

class ActivatedAbility : public Ability {
public:
    std::vector<std::reference_wrapper<Cost>> costs;
};

#endif
