#include "../ability.h"
#include "../card.h"
#include "../environment.h"

#include "../staticeffects/plusForEachEffect.h"

#include "cardManager.h"

Card TempestDjinn = newCard("Tempest Djinn", 3, {}, { CREATURE }, { DJINN }, 0, 4, 0, { BLUE }, Mana({ BLUE, BLUE, BLUE }), {},
	{}, {}, {}, { std::shared_ptr<StaticEffectHandler>(new PlusForEachHandler(BATTLEFIELD, [](std::shared_ptr<const Targetable> target, const Environment& env, xg::Guid controller)
																							-> bool {
		std::shared_ptr<const CardToken> card = std::dynamic_pointer_cast<const CardToken>(target);
		std::shared_ptr<const std::set<CardSuperType>> superTypes = env.getSuperTypes(card);
		std::shared_ptr<const std::set<CardSubType>> subTypes = env.getSubTypes(card);
		return env.getController(card) == controller && superTypes->find(BASIC) != superTypes->end() && subTypes->find(ISLAND) != subTypes->end();
	})) });

class TManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, TempestDjinn);
	}
};
