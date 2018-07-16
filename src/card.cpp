#include "card.h"
#include "targeting.h"

HasEffect::HasEffect(std::shared_ptr<TargetingRestriction> targeting)
	: targeting(targeting)
{}

CostedEffect::CostedEffect() {}

CostedEffect::CostedEffect(std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts,
						   std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>> source)
    : costs(costs), additionalCosts(additionalCosts), source(source)
{}

std::shared_ptr<Cost> CostedEffect::canPlay(Player& player, Environment& env) {
    // CodeReview: Can choose targets if *this is a HasEffect
    // CodeReview: Handle timing restrictions
    
    for(std::shared_ptr<Cost>& cost : this->costs) {
        if(cost->canPay(player, env, this->source)) return cost;
    }
    return std::shared_ptr<Cost>();
}

Changeset CardToken::applyEffect(const Environment& env) {
    Changeset change;
    for(auto& c : this->applyEffects) c(change, env, this->id);
    return change;
}

CardToken::CardToken()
	: HasEffect(std::shared_ptr<TargetingRestriction>(new NoTargets()))
{}

CardToken::CardToken(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
                     int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
                     std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
                     std::shared_ptr<TargetingRestriction> targeting,
					 std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyEffects)
    : HasEffect(targeting), baseSuperTypes(superTypes), baseTypes(types), baseSubTypes(subTypes), basePower(power),
	  baseToughness(toughness), startingLoyalty(loyalty), name(name), cmc(cmc), baseColors(colors),
	  activatableAbilities(activatedAbilities), applyEffects(applyEffects)
    {}

Card::Card() {}

Card::Card(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
           std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
		   std::shared_ptr<TargetingRestriction> targeting,
           std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
           std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                targeting, applyAbilities), CostedEffect(costs, additionalCosts, std::shared_ptr<Card>(this))
    {}

Token::Token(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
           std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
		   std::shared_ptr<TargetingRestriction> targeting,
           std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                targeting, applyAbilities)
    {}
