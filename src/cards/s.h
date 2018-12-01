#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "../abilities/counterUnless.h"
#include "../abilities/explore.h"
#include "../abilities/singleProliferate.h"
#include "../propositions/raid.h"
#include "../replacementeffects/entersWithCounters.h"
#include "../triggeredeffects/combatDamageTrigger.h"
#include "../staticeffects/conditionalCostReduction.h"

#include "cardManager.h"

Card SirenLookout = newCard("Siren Lookout", 3, {}, { CREATURE }, { SIREN, PIRATE }, 1, 2, 0, { BLUE }, Mana(2, { BLUE }), {}, {},
	{}, { std::shared_ptr<TriggerHandler>(new EtbTriggerHandler([](std::shared_ptr<CardToken> card, std::optional<xg::Guid>) ->
			std::shared_ptr<Ability> { return std::make_shared<Ability>(LambdaEffects(ExploreAbility(card->id)), std::shared_ptr<TargetingRestriction>(new NoTargets())); }, true)) });
Card SirenReaver = newCard("Siren Reaver", 4, {}, { CREATURE }, { SIREN, PIRATE }, 3, 2, 0, { BLUE }, Mana(3, { BLUE }), {}, {},
	{}, {}, { std::make_shared<ConditionalCostReduction>(RaidProposition(), ManaCost(Mana(1, {}))) });
Card SirenStormtamer = newCard("Siren Stormtamer", 1, {}, { CREATURE }, { SIREN, PIRATE, WIZARD }, 1, 1, 0, { BLUE }, Mana({ BLUE }), {},
	{ std::make_shared<ActivatedAbility>(LambdaEffects([](xg::Guid source, const Environment& env)->std::optional<Changeset> {
		Changeset change;
		std::shared_ptr<Targetable> target = env.gameObjects.at(env.targets.at(source)[0]);
		if (std::dynamic_pointer_cast<Card>(target)) {
			change.push_back(ObjectMovement{ target->id, env.stack->id, env.graveyards.at(target->owner)->id, COUNTER });
		}
		else {
			change.push_back(RemoveObject{ target->id, env.stack->id });
		}
		return change;
	}), std::make_shared<TargetsYouOrPermanentYouControlTarget>(), std::vector<CostValue>{ CombineCost(ManaCost(Mana({BLUE})), SacrificeCost()) }) });
Card SkyshipPlunderer = newCard("Skyship Plunderer", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 2, 1, 0, { BLUE }, Mana(1, { BLUE }), {}, {}, {},
	{ std::shared_ptr<TriggerHandler>(new CombatDamageTrigger(toPlayerProp, [](CombatDamageToTarget&) {return std::make_shared<Ability>(LambdaEffects(SingleProliferateAbility()), std::shared_ptr<TargetingRestriction>(new PermanentOrPlayerTarget())); })) });
Card SpellPierce = newCard("Spell Pierce", 1, {}, { INSTANT }, {}, 0, 0, 0, { BLUE }, std::make_shared<AndTarget<NonCreatureTarget, SpellTarget>>(), { ManaCost(Mana({ BLUE })) }, {},
	{ CounterUnlessAbility(ManaCost(Mana(2, {})), {}) });
Card StormFleetAerialist = newCard("Storm Fleet Aerialist", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 1, 2, 0, { BLUE }, Mana(1, { BLUE }), {},
	{}, { std::shared_ptr<EventHandler>(new EntersWithCounters(1, RaidProposition())) }, {}, {}, { 0 });
Card Swamp = newCard("Swamp", 0, std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{SWAMP}, 0, 0, 0,
	std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
	{ LandPlayCost() }, {},
	{},
	std::vector<std::shared_ptr<ActivatedAbility>>{
	std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{BLACK}),
		{ TapCost() }))});

class SManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, SirenLookout);
		insertCard(cards, SirenReaver);
		insertCard(cards, SirenStormtamer);
		insertCard(cards, SkyshipPlunderer);
		insertCard(cards, SpellPierce);
		insertCard(cards, StormFleetAerialist);
		insertCard(cards, Swamp);
	}
};
