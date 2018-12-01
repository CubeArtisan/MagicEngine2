#ifndef _EXPLORE_H_
#define _EXPLORE_H_

#include "../card.h"
#include "../changeset.h"
#include "../environment.h"

class ExploreAbility {
public:
	std::optional<Changeset> operator()(xg::Guid, const Environment& env) const {
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(env.gameObjects.at(env.getController(this->explorer)));
		std::shared_ptr<Zone<Card, Token>> library = env.libraries.at(player->id);
		std::shared_ptr<Zone<Card, Token>> graveyard = env.graveyards.at(player->id);
		std::shared_ptr<Zone<Card, Token>> hand = env.hands.at(player->id);

		// CodeReview: If library is empty don't reveal and just give a counter
		// CodeReview: Reveal to all players
		std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>> var = library->objects.back();
		std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(var);
		std::shared_ptr<const std::set<CardType>> types = env.getTypes(card);
		xg::Guid cardId = card->id;
		Changeset result;
		if (types->find(LAND) != types->end()) {
			result.push_back(ObjectMovement{ cardId, library->id, hand->id });
		}
		else {
			Changeset toLibrary(ObjectMovement{ cardId, library->id, library->id });
			Changeset toGraveyard(ObjectMovement{ cardId, library->id, graveyard->id });
			result = player->strategy->chooseOne({ toLibrary, toGraveyard }, *player, env);
			// CodeReview: Only if still on the battlefield
			result.push_back(AddPermanentCounter{ explorer, PLUSONEPLUSONECOUNTER, 1 });
		}
		return result;
	}

	ExploreAbility(xg::Guid explorer)
		: explorer(explorer)
	{}

private:
	xg::Guid explorer;
};

#endif