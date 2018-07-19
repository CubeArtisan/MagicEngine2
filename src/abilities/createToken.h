#ifndef _CREATETOKEN_H_
#define _CREATETOKEN_H_

#include <memory>

#include "../ability.h"
#include "../changeset.h"
#include "../targeting.h"

struct CreateTokensAbility : public Ability {
	Changeset applyEffect(const Environment& env) const {
		Changeset createTokens;
		for (int i = 0; i < this->amount; i++) {
			std::shared_ptr<Targetable> created = std::shared_ptr<Targetable>(new Token(token));
			created->id = xg::newGuid();
			// 110.5a. The player who creates a token is its owner. The token enters the battlefield under that player's control.
			created->owner = env.getController(this->id);
			createTokens.create.push_back(ObjectCreation{ env.battlefield->id, created });
		}
		return createTokens;
	}

	CreateTokensAbility(int amount, Token token)
		: Ability(std::shared_ptr<TargetingRestriction>(new NoTargets())), amount(amount), token(token)
	{}

	std::shared_ptr<Ability> clone() const {
		return std::shared_ptr<Ability>(new CreateTokensAbility(*this));
	}

private:
	const int amount;
	const Token token;
};

#endif