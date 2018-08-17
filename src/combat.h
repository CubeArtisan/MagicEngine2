#ifndef _COMBAT_H_
#define _COMBAT_H_

#include<set>
#include <typeinfo>

#include "card.h"
#include "util.h"

class AttackRestriction {
public:
	virtual std::set<xg::Guid> canAttack(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
										 const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
										 const Environment& env) const = 0;

	virtual bool operator==(const AttackRestriction& other) {
		return typeid(*this) == typeid(other);
	}
};

class AttackRestrictionValue : public polyValue<AttackRestriction>, public AttackRestriction {
public:
	std::set<xg::Guid> canAttack(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
		const Environment& env) const override {
		return this->value().canAttack(card, possibleAttacks, declaredAttacks, env);
	}

	bool operator==(const AttackRestrictionValue& other) const {
		return typeid(this->value()) == typeid(other.value());
	}

	using polyValue<AttackRestriction>::polyValue;
};

class AttackRequirement {
public:
	virtual std::set<xg::Guid> getRequiredAttacks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
										          const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
										          const Environment& env) const = 0;
};

class AttackRequirementValue : public polyValue<AttackRequirement>, public AttackRequirement {
public:
	virtual std::set<xg::Guid> getRequiredAttacks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleAttacks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredAttacks,
		const Environment& env) const {
		return this->value().getRequiredAttacks(card, possibleAttacks, declaredAttacks, env);
	}

	bool operator==(const AttackRequirementValue& other) const {
		return typeid(this->value()) == typeid(other.value());
	}

	using polyValue<AttackRequirement>::polyValue;
};

class BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
										const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
										const Environment& env) const = 0;
};

class BlockRestrictionValue : public polyValue<BlockRestriction>, public BlockRestriction {
public:
	virtual std::set<xg::Guid> canBlock(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
		const Environment& env) const override {
		return this->value().canBlock(card, possibleBlocks, declaredBlocks, env);
	}

	bool operator==(const BlockRestrictionValue& other) const {
		return typeid(this->value()) == typeid(other.value());
	}

	using polyValue<BlockRestriction>::polyValue;
};

class BlockRequirement {
public:
	virtual std::set<xg::Guid> getRequiredBlocks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
												 const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
												 const Environment& env) const = 0;
};

class BlockRequirementValue : public polyValue<BlockRequirement>, public BlockRequirement {
public:
	virtual std::set<xg::Guid> getRequiredBlocks(const std::shared_ptr<CardToken>& card, const std::set<xg::Guid>& possibleBlocks,
		const std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>>& declaredBlocks,
		const Environment& env) const override {
		return this->value().getRequiredBlocks(card, possibleBlocks, declaredBlocks, env);
	}

	bool operator==(const BlockRequirementValue& other) const {
		return typeid(this->value()) == typeid(other.value());
	}

	using polyValue<BlockRequirement>::polyValue;
};

#endif