#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "../abilities/singleProliferate.h"
#include "../abilities/explore.h"
#include "../replacementeffects/entersWithCounters.h"
#include "../propositions/raid.h"
#include "../triggeredeffects/combatDamageTrigger.h"
#include "../staticeffects/conditionalCostReduction.h"

#include "cardManager.h"

Card SirenLookout = newCard("Siren Lookout", 3, {}, { CREATURE }, { SIREN, PIRATE }, 1, 2, 0, { BLUE }, Mana(2, { BLUE }), {}, {},
	{}, { std::shared_ptr<TriggerHandler>(new EtbTriggerHandler([](std::shared_ptr<CardToken> card, std::optional<xg::Guid>) ->
			std::shared_ptr<Ability> { return std::make_shared<Ability>(LambdaEffects(ExploreAbility(card->id)), std::shared_ptr<TargetingRestriction>(new NoTargets())); }, true)) });
Card SirenReaver = newCard("Siren Reaver", 4, {}, { CREATURE }, { SIREN, PIRATE }, 3, 2, 0, { BLUE }, Mana(3, { BLUE }), {}, {},
	{}, {}, { std::make_shared<ConditionalCostReduction>(RaidProposition(), ManaCost(Mana(1, {}))) });
Card SkyshipPlunderer = newCard("Skyship Plunderer", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 2, 1, 0, { BLUE }, Mana(1, { BLUE }), {}, {}, {},
	{ std::shared_ptr<TriggerHandler>(new CombatDamageTrigger(toPlayerProp, [](DamageToTarget) {return std::make_shared<Ability>(LambdaEffects(SingleProliferateAbility()), std::shared_ptr<TargetingRestriction>(new PermanentOrPlayerTarget())); }))});
Card StormFleetAerialist = newCard("Storm Fleet Aerialist", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 1, 2, 0, { BLUE }, Mana(1, { BLUE }), {},
	{}, { std::shared_ptr<EventHandler>(new EntersWithCounters(1, RaidProposition())) }, {}, {}, { 0 });
Card Swamp = newCard("Swamp", 0, std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{SWAMP}, 0, 0, 0,
				     std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
					 { LandPlayCost() }, {},
				     {},
				     std::vector<std::shared_ptr<ActivatedAbility>>{
				  	     std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{BLACK}),
							 {TapCost()}))});

class SManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, SirenLookout);
		insertCard(cards, SkyshipPlunderer);
		insertCard(cards, StormFleetAerialist);
		insertCard(cards, Swamp);
	}
};
