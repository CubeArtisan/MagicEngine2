#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

class Ability : public Targetable, public HasEffect {
public:
    //CodeReview: Move onto Environment
    std::vector<std::reference_wrapper<Targetable>> targets;
    xg::Guid controller;
    
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;
    std::set<Color> colors;

    bool is_legal(int pos, Targetable& target);
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>);
};

class ActivatedAbility : public Ability {
public:
    std::vector<std::reference_wrapper<Cost>> costs;
};

#endif
