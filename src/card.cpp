#include "card.h"

Changeset CardToken::applyEffect(const Environment& env) {
    Changeset change;
    for(auto& c : this->applyEffects) c(change, env);
    return change;
}

CardToken::CardToken(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
                     int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
                     std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts,
                     std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
                     std::vector<std::function<Changeset&(Changeset&, const Environment&)>> applyEffects)
    : baseSuperTypes(superTypes), baseTypes(types), baseSubTypes(subTypes), basePower(power), baseToughness(toughness),
      startingLoyalty(loyalty), name(name), cmc(cmc), baseColors(colors), costs(costs), additionalCosts(additionalCosts),
      activatableAbilities(activatedAbilities), applyEffects(applyEffects)
    {}

Card::Card(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
           std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts,
           std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
           std::vector<std::function<Changeset&(Changeset&, const Environment&)>> applyAbilities)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, costs, additionalCosts,
                activatedAbilities, applyAbilities)
    {}

bool Card::canPlay(Player& player, Environment& env) {
    // CodeReview: Can choose targets
    
}
