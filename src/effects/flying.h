#ifndef _FLYING_H_
#define _FLYING_H_

#include "../changeset.h"
#include "../combat.h"
#include "../environment.h"

bool canBlockFlying(std::shared_ptr<CardToken> card, const Environment& env);

class FlyingRestriction : public BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>&,
		const Environment& env) const override {
		std::set<xg::Guid> result;
		for(auto& possibility : possibleBlocks){
			if (possibility == this->flyer && !canBlockFlying(card, env)) continue;
			result.insert(possibility);
		}
		return result;
	}

	FlyingRestriction(xg::Guid flyer)
		: flyer(flyer)
	{}

private:
	xg::Guid flyer;
};

class FlyingHandler : public StaticEffectHandler {
public:
	StaticEffectQuery& handleEvent(StaticEffectQuery& query, const Environment& env) const override {
		if (BlockRestrictionQuery* block = std::get_if<BlockRestrictionQuery>(&query)) {
			bool found = false;
			for (auto& attack : env.declaredAttacks) {
				if (attack.first->id == this->owner) {
					found = true;
					break;
				}
			}
			if (!found) return query;

			block->restrictions.push_back(FlyingRestriction(this->owner));
		}
	}

	FlyingHandler()
		: StaticEffectHandler({}, { BATTLEFIELD })
	{}
};

bool canBlockFlying(std::shared_ptr<CardToken> card, const Environment& env) {
	for (auto& ability : env.getStaticEffects(card, BATTLEFIELD, std::nullopt)) {
		if (std::dynamic_pointer_cast<FlyingHandler>(ability)) return true;
	}
	return false;
}
#endif