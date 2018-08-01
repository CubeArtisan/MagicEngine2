#ifndef _COMBAT_H_
#define _COMBAT_H_

#include<set>

#include "card.h"
#include "util.h"

class AttackRestriction {
public:
	virtual std::set<xg::Guid> canAttack(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
										 const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
										 const Environment& env) const = 0;
};

class AttackRestrictionValue : public polyValue<AttackRestriction>, AttackRestriction {
public:
	std::set<xg::Guid> canAttack(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
		const Environment& env) const override {
		return this->value().canAttack(card, possibleAttacks, declaredAttacks, env);
	}
};

class AttackRequirement {
public:
	virtual std::set<xg::Guid> getRequiredAttacks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
										          const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
										          const Environment& env) const = 0;
};

class AttackRequirementValue : public polyValue<AttackRequirement>, AttackRequirement {
public:
	virtual std::set<xg::Guid> getRequiredAttacks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
		const Environment& env) const {
		return this->value().getRequiredAttacks(card, possibleAttacks, declaredAttacks, env);
	}
};

class BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
										const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
										const Environment& env) const = 0;
};

class BlockRestrictionValue : public polyValue<BlockRestriction>, BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
		const Environment& env) const override {
		return this->value().canBlock(card, possibleBlocks, declaredBlocks, env);
	}
};

class BlockRequirement {
public:
	virtual std::set<xg::Guid> getRequiredBlocks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
												 const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
												 const Environment& env) const = 0;
};

class BlockRequirementValue : polyValue<BlockRequirement>, BlockRequirement {
public:
	virtual std::set<xg::Guid> getRequiredBlocks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
		const Environment& env) const override {
		return this->value().getRequiredBlocks(card, possibleBlocks, declaredBlocks, env);
	}
};

#endif