#include "cardManager.h"

#include "../propositions/attacking.h"
#include "../staticeffects/conditionalEffect.h"
#include "../staticeffects/flying.h"

Card KitesailCorsair = newCard("Kitesail Corsair", 2, {}, { CREATURE }, { HUMAN, PIRATE }, 2, 1, 0, { BLUE },
	Mana(1, { BLUE }), {}, {}, {}, {}, { std::make_shared<ConditionalEffectHandler>(AttackingProposition(), FlyingHandler()) });

class KManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, KitesailCorsair);
	}
};