#ifndef _COMBAT_H_
#define _COMBAT_H_

#include<set>

#include "dyno.hpp"
#include "guid.hpp"

struct CardToken;
struct Environment;

DYNO_INTERFACE(AttackRestriction,
	(canAttack, std::set<xg::Guid>(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
								   const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
								   const Environment& env) const)
);

class AttackRequirement {
public:
	virtual std::set<xg::Guid> getRequiredAttacks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
										          const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
										          const Environment& env) const = 0;
};

class BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
										const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
										const Environment& env) const = 0;
};

class BlockRequirement {
public:
	virtual std::set<xg::Guid> getRequiredBlocks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
												 const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
												 const Environment& env) const = 0;
};

#endif