#ifndef _RUNNER_H_
#define _RUNNER_H_

#include <variant>

#include "changeset.h"
#include "environment.h"

class Runner {
public:
	void runGame();

	Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players);
	bool applyChangeset(Changeset& changeset, bool replacementEffects = true, bool triggerEffects = true);
	std::optional<Changeset> checkStateBasedActions() const;
	// CodeReview: Implement
	void allowActivateManaAbility(xg::Guid player) const;

private:
	const std::vector<std::vector<Card>> libraries;
	const std::vector<Player> players;
	Environment env;
	std::optional<Changeset> executeStep() const;
	void applyMoveRules(Changeset& changeset);
	bool applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied = {});
};

#endif