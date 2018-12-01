#ifndef _CREATETOKEN_H_
#define _CREATETOKEN_H_

#include <memory>

#include "../ability.h"
#include "../changeset.h"
#include "../targeting.h"

struct CreateTokensAbility {
	std::optional<Changeset> operator()(xg::Guid source, const Environment& env) const {
		Changeset createTokens;
		for (int i = 0; i < this->amount; i++) {
			std::shared_ptr<Targetable> created = std::shared_ptr<Targetable>(new Token(token));
			created->id = xg::newGuid();
			// 110.5a. The player who creates a token is its owner. The token enters the battlefield under that player's control.
			created->owner = env.getController(source);
			createTokens.push_back(CreateObject{ env.battlefield->id, created });
		}
		return createTokens;
	}

	CreateTokensAbility(int amount, Token token)
		: amount(amount), token(token)
	{}

private:
	const int amount;
	const Token token;
};

#endif