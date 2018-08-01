#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "../propositions/raid.h"
#include "../effects/entersWithCounters.h"

#include "cardManager.h"

Card Swamp = newCard("Swamp", 0, std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{SWAMP}, 0, 0, 0,
				     std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
				     std::vector<std::shared_ptr<Cost>>{std::shared_ptr<Cost>(new LandPlayCost())}, std::vector<std::shared_ptr<Cost>>{},
				     std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{},
				     std::vector<std::shared_ptr<ActivatedAbility>>{
				  	     std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{BLACK}),
				  		     std::vector<std::shared_ptr<const Cost>>{std::shared_ptr<const Cost>(new TapCost())}))});
Card StormFleetAerialist = newCard("Storm Fleet Aerialist", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 1, 2, 0, { BLUE }, Mana(1, { BLUE }), {},
	{}, { std::shared_ptr<EventHandler>(new EntersWithCounters(1, RaidProposition())) }, {}, {}, { 0 });

class SManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, Swamp);
		insertCard(cards, StormFleetAerialist);
	}
};
