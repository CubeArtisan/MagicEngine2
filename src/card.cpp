#include "card.h"
#include "targeting.h"

HasEffect::HasEffect(std::shared_ptr<const TargetingRestriction> targeting)
	: targeting(targeting)
{}

CostedEffect::CostedEffect() {}

CostedEffect::CostedEffect(std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts,
						   std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>> source)
    : costs(costs), additionalCosts(additionalCosts), source(source)
{}

std::shared_ptr<const Cost> CostedEffect::canPlay(const Player& player, const Environment& env) const {
    // CodeReview: Can choose targets if *this is a HasEffect - Might not be neccesary
	// CodeReview: Check are in a valid zone to be played from
	if (const Card* self = dynamic_cast<const Card*>(this)) {
		if (!env.goodTiming(self->id)) return std::shared_ptr<Cost>();
	}
	// CodeReview: Figure out timing for abilities

    for(const std::shared_ptr<const Cost>& cost : this->costs) {
        if(cost->canPay(player, env, this->source)) return cost;
    }
    return std::shared_ptr<Cost>();
}

Changeset CardToken::applyEffect(const Environment& env) const {
    Changeset change;
    for(auto& c : this->applyEffects) c(change, env, this->id);
    return change;
}

CardToken::CardToken()
	: CardToken(std::shared_ptr<std::set<CardSuperType>>(new std::set<CardSuperType>{}), std::shared_ptr<std::set<CardType>>(new std::set<CardType>{}),
		std::shared_ptr<std::set<CardSubType>>(new std::set<CardSubType>{}), 0,
		0, 0, "Unnamed", 0, std::set<Color>{},
		std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>>(new std::vector<std::shared_ptr<const ActivatedAbility>>{}),
		std::shared_ptr<TargetingRestriction>(new NoTargets()),
		std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{})
{}

CardToken::CardToken(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
					 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
                     int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
					 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
                     std::shared_ptr<const TargetingRestriction> targeting,
					 std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyEffects)
    : HasEffect(targeting), baseSuperTypes(superTypes), baseTypes(types), baseSubTypes(subTypes), basePower(power),
	  baseToughness(toughness), startingLoyalty(loyalty), name(name), cmc(cmc), baseColors(colors),
	  activatableAbilities(activatedAbilities), applyEffects(applyEffects)
    {}

Card::Card() {}

Card::Card(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
	       std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
		   std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		   std::shared_ptr<const TargetingRestriction> targeting,
           std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
           std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                targeting, applyAbilities), CostedEffect(costs, additionalCosts, std::shared_ptr<Card>())
    {}

Token::Token(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
			 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
             int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
			 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		     std::shared_ptr<const TargetingRestriction> targeting,
             std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                targeting, applyAbilities)
    {}
