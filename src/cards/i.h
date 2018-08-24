#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "../abilities/damageAbility.h"
#include "../triggeredeffects/etb.h"

#include "cardManager.h"


Card ImpactTremors = newCard("Impact Tremors", 2, {}, { ENCHANTMENT }, {}, 0, 0, 0,
							 { RED }, std::shared_ptr<TargetingRestriction>(new NoTargets()), Mana(1, { RED }), {},
	 {}, {}, { std::shared_ptr<TriggerHandler>(new EtbTriggerHandler([](std::shared_ptr<CardToken>, std::optional<xg::Guid>) -> std::shared_ptr<Ability> { return std::make_shared<Ability>(LambdaEffects(EqualDamageEachOpponentAbility(1)), std::shared_ptr<TargetingRestriction>(new NoTargets())); })) });

Card Island = newCard("Island", 0, std::set<CardSuperType>{BASIC}, std::set<CardType>{LAND}, std::set<CardSubType>{ISLAND}, 0, 0, 0,
				      std::set<Color>{}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
					  {LandPlayCost()}, {},
					  {},
					  std::vector<std::shared_ptr<ActivatedAbility>>{
					  std::shared_ptr<ActivatedAbility>(new ManaAbility(Mana(std::multiset<Color>{BLUE}),
					  {TapCost()}))});

class IManager : public LetterManager {
public:
    void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
    {
		insertCard(cards, ImpactTremors);
        cards.insert(std::make_pair(Island.name, Island));
    }
};
