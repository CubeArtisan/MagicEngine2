#include "cardManager.h"
#include "../environment.h"
#include "../targeting.h"
#include "../abilities/createToken.h"
#include "../triggeredeffects/etb.h"

Card GoblinInstigator = newCard("Goblin Instigator", 2, std::set<CardSuperType>{}, { CREATURE }, { GOBLIN }, 1, 1, 0,
	{RED}, std::shared_ptr<TargetingRestriction>(new NoTargets()),
	Mana(1, { RED }), {}, {}, {}, { std::shared_ptr<TriggerHandler>(new EtbTriggerHandler(
		[](std::shared_ptr<CardToken>, std::optional<xg::Guid>)->
			std::shared_ptr<Ability> { return std::make_shared<Ability>(LambdaEffects(CreateTokensAbility(1, tokenManager.getToken("Goblin"))), std::shared_ptr<TargetingRestriction>(new NoTargets)); }, true)) });

class GManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, GoblinInstigator);
	}
};
