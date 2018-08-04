#ifndef _GAMEACTION_H_
#define _GAMEACTION_H_

#include <memory>
#include <variant>
#include <vector>

struct Card;
struct Cost;
struct ActivatedAbility;
struct Token;
struct Emblem;

// CodeReview: Only handles casting from hand currently
// CodeReview: Implement Modes
struct CastSpell {
	const xg::Guid spell;
	const std::vector<xg::Guid> targets;
	const Cost& cost;
	const std::vector<std::shared_ptr<Cost>> additionalCosts;
	const unsigned int x;
};

// CodeReview: Support playing lands from other zones
struct PlayLand {
	const xg::Guid land;
};

struct ActivateAnAbility {
	const std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>, std::shared_ptr<const Emblem>> source;
	const std::shared_ptr<ActivatedAbility> ability;
	const std::vector<xg::Guid> targets;
	const Cost& cost;
	const unsigned int x;
};

struct PassPriority {};

using GameAction = std::variant<CastSpell, PlayLand, ActivateAnAbility, PassPriority>;

#endif
