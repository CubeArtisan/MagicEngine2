#include "stepOrPhase.h"

StepOrPhase::StaticInitializer::StaticInitializer()
	: steps{
		{UNTAP, std::shared_ptr<StepOrPhase>(new UntapStep())},
		{UPKEEP, std::shared_ptr<StepOrPhase>(new UpkeepStep())},
		{DRAW, std::shared_ptr<StepOrPhase>(new DrawStep())},
		{PRECOMBATMAIN, std::shared_ptr<StepOrPhase>(new PreCombatMainPhase())},
		{BEGINCOMBAT, std::shared_ptr<StepOrPhase>(new BeginCombatStep())},
		{DECLAREATTACKERS, std::shared_ptr<StepOrPhase>(new DeclareAttackersStep())},
		{DECLAREBLOCKERS, std::shared_ptr<StepOrPhase>(new DeclareBlockersStep())},
		{FIRSTSTRIKEDAMAGE, std::shared_ptr<StepOrPhase>(new FirstStrikeDamageStep())},
		{COMBATDAMAGE, std::shared_ptr<StepOrPhase>(new CombatDamageStep())},
		{ENDCOMBAT, std::shared_ptr<StepOrPhase>(new EndCombatStep())},
		{POSTCOMBATMAIN, std::shared_ptr<StepOrPhase>(new PostCombatMainPhase())},
		{END, std::shared_ptr<StepOrPhase>(new EndStep())},
		{CLEANUP, std::shared_ptr<StepOrPhase>(new CleanupStep())}
}
{}

bool UntapStep::applyEnter(Environment& env, Runner& runner) const {
	xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
	env.landPlays[turnPlayerId] = 0;

	// CodeReview: Implement Phasing
	// 502.1. First, all phased-in permanents with phasing that the active player controls phase out, and all
	// phased-out permanents that the active player controlled when they phased out phase in. This all happens
	// simultaneously. This turn-based action doesn't use the stack.

	// 502.2. Second, the active player determines which permanents they control will untap. Then they untap
	// them all simultaneously. This turn-based action doesn't use the stack. Normally, all of a player's
	// permanents untap, but effects can keep one or more of a player's permanents from untapping.

	// CodeReview: Implemented restricted untap system
	// Get a set of filters/restrictions(card -> vector<card> -> bool) that says whether it can untap given all the other things that are untapping
	// Get the set of all remaining cards that could untap given what's already untapping
	// Have the strategy pick one to untap
	// Repeat until remaining cards to untap is empty
	Changeset untap;
	untap.changes.reserve(env.battlefield->objects.size());
	for (auto& object : env.battlefield->objects) {
		std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(object);
		if (env.getController(card->id) == turnPlayerId) {
			std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(card->id))->isSummoningSick = false;
			if (card->isTapped) {
				untap.changes.push_back(std::shared_ptr<GameChange>(new TapTarget{ card->id, false }));
			}
		}
	}

	// 502.3.No player receives priority during the untap step, so no spells can be cast or resolve and no
	// abilities can be activated or resolve.Any ability that triggers during this step will be held until
	// the next time a player would receive priority, which is usually during the upkeep step.
	untap.changes.push_back(std::shared_ptr<GameChange>(new StepOrPhaseChange{ UNTAP }));
#ifdef DEBUG
	std::cout << "Untapping permanents for " << turnPlayerId << std::endl;
#endif
	return runner.applyChangeset(untap);
}

bool DrawStep::applyEnter(Environment& env, Runner& runner) const {
	xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
	// 504.1. First, the active player draws a card. This turn-based action doesn't use the stack.
	// CodeReview: Handle first turn don't draw
	Changeset drawCard = Changeset::drawCards(turnPlayerId, 1, env);
#ifdef DEBUG
	auto moves = drawCard.ofType<ObjectMovement>();
	for (const std::shared_ptr<ObjectMovement>& move : moves) {
		if (move->sourceZone != env.libraries.at(turnPlayerId)->id || move->destinationZone != env.hands.at(turnPlayerId)->id) continue;
		std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(move->object));
		std::cout << env.players[env.turnPlayer]->id << "is drawing " << card->name << std::endl;
	}
#endif
	return runner.applyChangeset(drawCard);
}

bool PreCombatMainPhase::applyEnter(Environment&, Runner&) const {
	// CodeReview: If archenemy do a scheme
	// 505.3. First, but only if the players are playing an Archenemy game (see rule 904), the active player is
	// the archenemy, and it's the active player's precombat main phase, the active player sets the top card of
	// their scheme deck in motion (see rule 701.24). This turn-based action doesn't use the stack.
	// CodeReview: Implement Saga's turn based action
	// 505.4. Second, if the active player controls one or more Saga enchantments and it's the active player's
	// precombat main phase, the active player puts a lore counter on each Saga they control. (See rule 714, "Saga Cards.")
	// This turn-based action doesn't use the stack.
	return true;
}

bool BeginCombatStep::applyEnter(Environment&, Runner&) const {
	// 507.1. First, if the game being played is a multiplayer game in which the active player's opponents don't
	// all automatically become defending players, the active player chooses one of their opponents. That player
	// becomes the defending player. This turn-based action doesn't use the stack.
	// CodeReview: Appoint defending. Should always be automatic
	return true;
}

bool DeclareAttackersStep::applyEnter(Environment& env, Runner& runner) const {
	std::set<xg::Guid> opponentsAndPlaneswalkers;
	xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
	Player& turnPlayer = *env.players[env.turnPlayer];
	for (auto& player : env.players) {
		if (player->id != turnPlayerId) opponentsAndPlaneswalkers.insert(player->id);
		for (auto& card : env.battlefield->objects) {
			std::shared_ptr<const CardToken> c = getBaseClassPtr<const CardToken>(card);
			if (env.getController(c) == turnPlayerId) continue;
			std::shared_ptr<const std::set<CardType>> types = env.getTypes(c);
			if (types->find(PLANESWALKER) != types->end()) opponentsAndPlaneswalkers.insert(c->id);
		}
	}
	std::map<std::shared_ptr<CardToken>, xg::Guid> declaredAttacks;
	std::set<xg::Guid> declaredAttackers;
	Changeset declareAttacks;
	do {
		std::vector<std::shared_ptr<CardToken>> possibleAttackers;
		std::map<xg::Guid, std::set<xg::Guid>> possibleAttacks;
		std::map<xg::Guid, std::multiset<xg::Guid>> requiredAttacks;
		for (auto& card : env.battlefield->objects) {
			std::shared_ptr<CardToken> c = std::dynamic_pointer_cast<CardToken>(env.gameObjects[getBaseClassPtr<const Targetable>(card)->id]);
			if (declaredAttackers.find(c->id) != declaredAttackers.end()) continue;
			if (env.getController(c) != turnPlayerId) continue;
			if (!env.canAttack(c)) continue;
			possibleAttacks[c->id] = opponentsAndPlaneswalkers;
			// We ignore cost based restrictions for now. Up in the air how to decide them.
			auto restrictions = env.getAttackRestrictions(c);
			// CodeReview: If any restriction is CantAttackAloneRestriction and there is another creature that can attack or has the same restriction make an entry for both
			for (auto& restriction : restrictions) {
				possibleAttacks[c->id] = restriction.canAttack(c, possibleAttacks[c->id], declaredAttacks, env);
			}
			if (possibleAttacks[c->id].empty()) continue;
			requiredAttacks[c->id] = {};
			auto requirements = env.getAttackRequirements(c);
			for (auto& requirement : requirements) {
				auto attacks = requirement.getRequiredAttacks(c, possibleAttacks[c->id], declaredAttacks, env);
				requiredAttacks[c->id].insert(attacks.begin(), attacks.end());
			}
			if (!requiredAttacks[c->id].empty()) {
				std::set<std::pair<size_t, xg::Guid>> opponentRequirements;
				for (const xg::Guid& guid : possibleAttacks[c->id]) {
					opponentRequirements.insert(std::make_pair(requiredAttacks[c->id].count(guid), guid));
				}
				std::set<xg::Guid> validAttacks;
				size_t max = opponentRequirements.rbegin()->first;
				for (auto iter = opponentRequirements.rbegin(); iter != opponentRequirements.rend(); iter++) {
					if (iter->first != max) break;
					validAttacks.insert(iter->second);
				}
				possibleAttacks[c->id] = validAttacks;
			}
			possibleAttackers.push_back(c);
		}
		// CodeReview: Need to figure out how to deal with costs to attack
		std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> attacker = turnPlayer.strategy->chooseAttacker(possibleAttackers, possibleAttacks, requiredAttacks, declaredAttacks);
		if (attacker) {
			declaredAttacks.insert(attacker.value());
			declaredAttackers.insert(attacker.value().first->id);
			declareAttacks.changes.push_back(std::shared_ptr<GameChange>(new DeclareAttack{ attacker.value().first->id, attacker.value().second }));
		}
		else break;
	} while (true);
	// CodeReview: Have the attacking player organize attackers into bands.
	for (auto& p : declaredAttacks) {
		// CodeReview: Use the StateQuery system to determine whether this is appropriate
		p.first->isTapped = true;
	}
	// CodeReview: Implement costs to attack
	// If there are optional cost associated with attacking "pay as attacks" strategy chooses whether to pay those
	// For each creature determine the total cost to attack. Sum these costs(can put into vector).
	// If these costs contain mana abilities allow the player to activate mana abilities
	// The active player pays all possible costs, if they cannot pay restart the process
	if (!declaredAttacks.empty()) {
		runner.applyChangeset(declareAttacks);
#ifdef DEBUG
		std::cout << turnPlayerId << " is declaring attacks" << std::endl;
		std::cout << declareAttacks << std::endl;
#endif
	}
	env.declaredAttacks = declaredAttacks;
	// CodeReview: Queue an event for declaring attackers
	// CodeReview: If there are no attackers skip to end of combat
	return true;
}

bool DeclareBlockersStep::applyEnter(Environment& env, Runner&) const {
	xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
	for (auto& player : env.players) {
		if (player->id == turnPlayerId) continue;
		std::set<xg::Guid> attackers;
		for (auto& attacker : env.declaredAttacks) {
			if (attacker.second == player->id || env.getController(attacker.second) == player->id)
				attackers.insert(attacker.first->id);
		}
		std::multimap<std::shared_ptr<CardToken>, xg::Guid> declaredBlocks;
		std::set<xg::Guid> declaredBlockers;
		do {
			std::vector<std::shared_ptr<CardToken>> possibleBlockers;
			std::map<xg::Guid, std::set<xg::Guid>> possibleBlocks;
			std::map<xg::Guid, std::multiset<xg::Guid>> requiredBlocks;
			for (auto& card : env.battlefield->objects) {
				std::shared_ptr<CardToken> c = std::dynamic_pointer_cast<CardToken>(env.gameObjects[getBaseClassPtr<const Targetable>(card)->id]);
				// CodeReview: Handle multiBlocking
				if (declaredBlockers.find(c->id) != declaredBlockers.end()) continue;
				if (env.getController(c) != player->id) continue;
				if (!env.canBlock(c)) continue;
				possibleBlocks[c->id] = attackers;
				// We ignore cost based restrictions for now. Up in the air how to decide them.
				auto restrictions = env.getBlockRestrictions(c);
				// If any restriction is CantBlockAloneRestriction and there is another creature that has the same restriction make an entry for both
				for (auto& restriction : restrictions) {
					possibleBlocks[c->id] = restriction.canBlock(c, possibleBlocks[c->id], declaredBlocks, env);
				}
				if (possibleBlocks[c->id].empty()) continue;
				requiredBlocks[c->id] = {};
				auto requirements = env.getBlockRequirements(c);
				for (auto& requirement : requirements) {
					auto blocks = requirement.getRequiredBlocks(c, possibleBlocks[c->id], declaredBlocks, env);
					requiredBlocks[c->id].insert(blocks.begin(), blocks.end());
				}
				if (!requiredBlocks[c->id].empty()) {
					std::set<std::pair<size_t, xg::Guid>> blockersRequirements;
					for (const xg::Guid& guid : possibleBlocks[c->id]) {
						blockersRequirements.insert(std::make_pair(requiredBlocks[c->id].count(guid), guid));
					}
					std::set<xg::Guid> validBlocks;
					size_t max = blockersRequirements.rbegin()->first;
					for (auto iter = blockersRequirements.rbegin(); iter != blockersRequirements.rend(); iter++) {
						if (iter->first != max) break;
						validBlocks.insert(iter->second);
					}
					possibleBlocks[c->id] = validBlocks;
				}
				possibleBlockers.push_back(c);
			}
			// CodeReview: Need to figure out how to deal with costs to attack
			std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> blocker = player->strategy->chooseBlocker(possibleBlockers, possibleBlocks, requiredBlocks, declaredBlocks);
			if (blocker) {
				declaredBlocks.insert(blocker.value());
				declaredBlockers.insert(blocker.value().first->id);
				env.blocked.insert(blocker.value().second);
			}
			else break;
		} while (true);
		// CodeReview: Have the defending player organize defenders into bands.
		// CodeReview: Create an event for notifying that blockers are declared

		// CodeReview: Implement costs to block
		// If there are optional cost associated with blocking "pay as blocks" strategy chooses whether to pay those
		// For each creature determine the total cost to block. Sum these costs(can put into vector).
		// If these costs contain mana abilities allow the player to activate mana abilities
		// The chosen player pays all possible costs, if they cannot pay restart the process

		env.declaredBlocks.insert(declaredBlocks.begin(), declaredBlocks.end());
		// CodeReview: Queue an event for declaring blockers
		// CodeReview: Tell the environment that creatures which have a blocker assigned to them are blocked
	}
	for (auto& attacker : env.declaredAttacks) {
		std::vector<std::shared_ptr<CardToken>> blockedBy;
		for (auto& blocker : env.declaredBlocks) {
			if (blocker.second == attacker.first->id) blockedBy.push_back(blocker.first);
		}
		if (blockedBy.empty()) continue;
		Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects[env.getController(attacker.first)]);
		env.blockingOrder[attacker.first->id] = player.strategy->chooseBlockingOrder(attacker.first, blockedBy, env);
	}
	// CodeReview: For each blocking creature its controller chooses damage assignment order if multiple blockees
	// CodeReview: If any attacking or blocking creatures have first or double strike create a FirstStrikeDamageStep
	return true;
}

bool DamageStep::chooseDamageAssignment(Environment& env, const std::shared_ptr<CardToken>& aggressor, Changeset& damageEvent) const {
	Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects[env.getController(aggressor)]);
	int powerRemaining = env.getPower(aggressor);
	for (xg::Guid& defender : env.blockingOrder[aggressor->id]) {
		int minDamage = std::min(env.getLethalDamage(aggressor, defender), powerRemaining);
		unsigned int damageAmount = player.strategy->chooseDamageAmount(aggressor, defender, minDamage, powerRemaining, env);
		powerRemaining -= damageAmount;
		damageEvent.changes.push_back(std::shared_ptr<GameChange>(new CombatDamageToTarget{ defender, damageAmount, aggressor->id }));
	}
	return true;
}

bool DamageStep::assignDamage(Environment& env, Runner& runner, const std::vector<std::shared_ptr<CardToken>>& attackers, const std::vector<std::shared_ptr<CardToken>>& blockers) const {
	Changeset damageEvent;
	for (const std::shared_ptr<CardToken>& attacker : attackers) {
		if (env.blocked.find(attacker->id) != env.blocked.end()) {
			this->chooseDamageAssignment(env, attacker, damageEvent);
		}
		else {
			damageEvent.changes.push_back(std::shared_ptr<GameChange>(new DamageToTarget{ env.declaredAttacks.at(attacker), (unsigned int)std::max(env.getPower(attacker), 0), attacker->id }));
		}
	}
	for (const std::shared_ptr<CardToken>& blocker : blockers) {
		this->chooseDamageAssignment(env, blocker, damageEvent);
	}
	return runner.applyChangeset(damageEvent);
}

bool FirstStrikeDamageStep::applyEnter(Environment&, Runner&) const {
	// CodeReview: Implement
	// CodeReview: Mark all creatures that dealt damage as having dealt damage this combat
	// CodeReview: For this phase only consider creatures with first or double strike
	return true;
}

bool CombatDamageStep::applyEnter(Environment& env, Runner& runner) const {
	Changeset damageEvent;
	std::vector<std::shared_ptr<CardToken>> attackers;
	std::set<std::shared_ptr<CardToken>> defenders;
	for (const std::pair<std::shared_ptr<CardToken>, xg::Guid>& attack : env.declaredAttacks) {
		attackers.push_back(attack.first);
		for (const std::pair<std::shared_ptr<CardToken>, xg::Guid>& block : env.declaredBlocks) {
			if (attack.first->id == block.second) defenders.insert(block.first);
		}
	}
	return this->assignDamage(env, runner, attackers, std::vector<std::shared_ptr<CardToken>>(defenders.begin(), defenders.end()));
	// CodeReview:For this phase only consider creatures that either have not dealt combat damage this round or have double strike
	// CodeReview: If multiple attackers(similar for blockers) would damage the same creature the total has to be lethal to continue in blocking order but not the individual parcels
}

bool EndCombatStep::applyLeave(Environment& env, Runner&) const {
	env.declaredAttacks.clear();
	env.declaredBlocks.clear();
	env.blocked.clear();
	env.blockingOrder.clear();
	return true;
}

bool CleanupStep::applyEnter(Environment& env, Runner& runner) const {
	xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
	auto handObjects = env.hands.at(turnPlayerId)->objects;
	// CodeReview: Do this with Hand Size from StateQuery
	// 514.1. First, if the active player's hand contains more cards than their maximum hand size (normally
	// seven), they discard enough cards to reduce their hand size to that number. This turn-based action doesn't
	// use the stack.
	if (handObjects.size() > 7) {
		Changeset discard = Changeset::discardCards(turnPlayerId, handObjects.size() - 7, env);
#ifdef DEBUG
		std::cout << turnPlayerId << " is discarding " << handObjects.size() - 7 << " cards" << std::endl;
#endif
		runner.applyChangeset(discard);
	}

	// 514.2. Second, the following actions happen simultaneously : all damage marked on permanents(including
	// phased - out permanents) is removed and all "until end of turn" and "this turn" effects end.This turn
	// - based action doesn't use the stack.
	// CodeReview: Send an end turn event through the system to get end of turn abilities to cleanup and any
	// relevant triggers
	// Should also move this into that changeset
	env.damage.clear();

	// 514.3a. At this point, the game checks to see if any state-based actions would be performed and/or any
	// triggered abilities are waiting to be put onto the stack (including those that trigger "at the beginning
	// of the next cleanup step"). If so, those state-based actions are performed, then those triggered abilities
	// are put on the stack, then the active player gets priority. Players may cast spells and activate abilities.
	// Once the stack is empty and all players pass in succession, another cleanup step begins.
	bool repeat = false;
	std::optional<Changeset> sba = runner.checkStateBasedActions();
	while (sba) {
		repeat = true;
		runner.applyChangeset(*sba);
		sba = runner.checkStateBasedActions();
	}
	if (!env.triggers.empty()) {
		repeat = true;
		// CodeReview: If triggers are queued empty the queue and mark repeat = true
	}
	if (repeat) {
		// CodeReview: Queue up another cleanup step
	}
	// else {
	Changeset cleanup;
	cleanup.changes.push_back(std::shared_ptr<GameChange>(new StepOrPhaseChange{ CLEANUP }));
	runner.applyChangeset(cleanup);
	// }
	return true;
}

bool CleanupStep::applyLeave(Environment& env, Runner&) const {
	// CodeReview: Get next player from Environment
	size_t nextPlayer = (env.turnPlayer + 1) % env.players.size();
	env.currentPlayer = nextPlayer;
	env.turnPlayer = nextPlayer;
	env.turnNumber++;
	return true;
}