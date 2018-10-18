#ifndef _GAMEACTION_H_
#define _GAMEACTION_H_

#include <memory>
#include <variant>
#include <vector>

struct Card;
struct CostValue;
struct ActivatedAbility;
struct Token;
struct Emblem;

// CodeReview: Only handles casting from hand currently
// CodeReview: Implement Modes
struct CastSpell {
	xg::Guid spell;
	std::vector<xg::Guid> targets;
	CostValue cost;
	std::vector<CostValue> additionalCosts;
	unsigned int x;
};

// CodeReview: Support playing lands from other zones
struct PlayLand {
	xg::Guid land;
};

struct ActivateAnAbility {
	SourceType source;
	std::shared_ptr<ActivatedAbility> ability;
	std::vector<xg::Guid> targets;
	CostValue cost;
	unsigned int x;
};

struct PassPriority {};

using GameAction = std::variant<CastSpell, PlayLand, ActivateAnAbility, PassPriority>;

#endif
