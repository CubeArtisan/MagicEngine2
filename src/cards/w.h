#include "../ability.h"
#include "../card.h"
#include "../environment.h"
#include "../staticeffects/flying.h"

#include "cardManager.h"

Card WindDrake = newCard("Wind Drake", 3, {}, {CREATURE}, {DRAKE}, 2, 2, 0,
	std::set<Color>{BLUE},
	Mana(2, { BLUE }), {}, {}, {}, {}, { std::shared_ptr<StaticEffectHandler>(new FlyingHandler()) });

class WManager : public LetterManager {
public:
	void getCards(std::map<std::string, Card>& cards, std::map<int, std::string>&)
	{
		insertCard(cards, WindDrake);
	}
};
