#include <iostream>
#include <sstream>
#include <string>

#include "ability.h"
#include "changeset.h"
#include "environment.h"
#include "runner.h"

Targetable::Targetable()
	: id(xg::newGuid())
{}

std::optional<std::vector<Changeset>> TriggerHandler::handleEvent(Changeset& changeset, const Environment& env) const {
	std::vector<Changeset> result;
	std::vector<QueueTrigger> queue = this->createTriggers(changeset, env);
	if (queue.empty()) return std::nullopt;
	Changeset createTrigger;
	for (QueueTrigger& q : queue) createTrigger.changes.push_back(std::shared_ptr<GameChange>{ new QueueTrigger(q) });
	result.push_back(createTrigger);
	return result;
}

bool ObjectMovement::ApplyTo(Environment& env, Runner& runner) {
	// CodeReview: Should we create a vector of all removes and all adds and do them that way
	std::shared_ptr<Targetable> objectUnwrapped = env.gameObjects.at(this->object);
	if (!objectUnwrapped) return false;
	Changeset remove;
	remove.changes.push_back(std::shared_ptr<GameChange>{ new RemoveObject{ this->object, this->sourceZone } });
	bool removalResult = runner.applyChangeset(remove);
	if (!removalResult) return false;
	Changeset create;
	objectUnwrapped->id = this->newObject;
	create.changes.push_back(std::shared_ptr<GameChange>{ new CreateObject{ this->destinationZone, objectUnwrapped, this->index } });
	return runner.applyChangeset(create);
}

std::string ObjectMovement::ToString() const {
	std::ostringstream os;
	os << "Movement: " << this->object << " from " << this->sourceZone << " to " << this->destinationZone
		<< " new GUID " << this->newObject << std::endl;
	return os.str();
}

bool AddPlayerCounter::ApplyTo(Environment& env, Runner&) {
	if (this->amount < 0 && env.playerCounters[this->player][this->counterType] < (unsigned int)-this->amount) {
		this->amount = -(int)env.playerCounters[this->player][this->counterType];
	}
	env.playerCounters[this->player][this->counterType] += this->amount;
	return true;
}

std::string AddPlayerCounter::ToString() const {
	std::ostringstream os;
	os << "Changing " << this->counterType << " counters on player " << this->player
		<< " by " << this->amount;
	return os.str();
}

bool AddPermanentCounter::ApplyTo(Environment& env, Runner&) {
	if (this->amount < 0 && env.permanentCounters[this->target][this->counterType] < (unsigned int)-this->amount) {
		this->amount = -(int)env.permanentCounters[this->target][this->counterType];
	}
	env.permanentCounters[this->target][this->counterType] += this->amount;
	return true;
}

std::string AddPermanentCounter::ToString() const {
	std::ostringstream os;
	os << "Changing " << this->counterType << " counters on permanent " << this->target
		<< " by " << this->amount;
	return os.str();
}

bool CreateObject::ApplyTo(Environment& env, Runner&) {
	if (!this->created) {
#ifdef DEBUG
		std::cout << "Creating a null object" << std::endl;
#endif
		return false;
	}
	std::shared_ptr<ZoneInterface> zoneUnwrapped = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects[this->zone]);
	xg::Guid id = this->created->id;
	zoneUnwrapped->addObject(this->created, this->index);
	env.gameObjects[id] = this->created;
	if (std::shared_ptr<CardToken> abilities = std::dynamic_pointer_cast<CardToken>(this->created)) {
		abilities->isSummoningSick = true;
		std::vector<std::shared_ptr<const EventHandler>> replacement = env.getReplacementEffects(abilities, zoneUnwrapped->type);
		std::vector<std::shared_ptr<const EventHandler>> replacements;
		replacements.reserve(replacement.size());
		for (auto& r : replacement) {
			std::shared_ptr<EventHandler> res = r->clone();
			res->owner = id;
			replacements.push_back(res);
		}
		env.replacementEffects.insert(env.replacementEffects.end(), replacements.begin(), replacements.end());
		std::vector<std::shared_ptr<const TriggerHandler>> trigger = env.getTriggerEffects(abilities, zoneUnwrapped->type);
		std::vector<std::shared_ptr<const TriggerHandler>> triggers;
		triggers.reserve(trigger.size());
		for (auto& t : trigger) {
			std::shared_ptr<TriggerHandler> trig = t->clone();
			trig->owner = id;
			triggers.push_back(trig);
		}
		env.triggerHandlers.insert(env.triggerHandlers.end(), triggers.begin(), triggers.end());
	}
	return true;
}

std::string CreateObject::ToString() const {
	std::ostringstream os;
	os << "Creating " << this->created->id << " in " << this->zone << " at index " << this->index;
	return os.str();
}

std::string CreateObject::ToString(Environment& env) const {
	std::shared_ptr<ZoneInterface> zoneUnwrapped = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects[this->zone]);
	std::ostringstream os;
	os << "Creating " << this->created->id << " in " << zoneUnwrapped->type << " at index " << this->index;
	return os.str();
}

bool RemoveObject::ApplyTo(Environment& env, Runner&) {
	std::shared_ptr<ZoneInterface> zoneUnwrapped = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects[this->zone]);
	if (zoneUnwrapped->findObject(this->object)) {
		zoneUnwrapped->removeObject(this->object);
	}
	else {
#ifdef DEBUG
		std::cout << "Tried to remove a nonexistent object from zone" << std::endl;
#endif
		throw "Failed to remove object";
	}
	env.gameObjects.erase(this->object);
	env.triggerHandlers.erase(std::remove_if(env.triggerHandlers.begin(), env.triggerHandlers.end(), [&](std::shared_ptr<const TriggerHandler>& a) -> bool { return a->owner == this->object; }), env.triggerHandlers.end());
	env.replacementEffects.erase(std::remove_if(env.replacementEffects.begin(), env.replacementEffects.end(), [&](std::shared_ptr<const EventHandler>& a) -> bool { return a->owner == this->object; }), env.replacementEffects.end());
	env.stateQueryHandlers.erase(std::remove_if(env.stateQueryHandlers.begin(), env.stateQueryHandlers.end(), [&](std::shared_ptr<const StaticEffectHandler>& a) -> bool { return a->owner == this->object; }), env.stateQueryHandlers.end());

	return true;
}

std::string RemoveObject::ToString() const {
	std::ostringstream os;
	os << "Removing " << this->object << " from " << this->zone;
	return os.str();
}

std::string RemoveObject::ToString(Environment& env) const {
	std::shared_ptr<ZoneInterface> zoneUnwrapped = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects[this->zone]);
	std::ostringstream os;
	os << "Removing " << this->object << " from " << zoneUnwrapped->type;
	return os.str();
}

bool LifeTotalChange::ApplyTo(Environment& env, Runner&) {
	env.lifeTotals[this->player] += this->amount;
	return true;
}

std::string LifeTotalChange::ToString() const {
	std::ostringstream os;
	os << "Adding " << this->amount << " to player " << this->player << "'s life total";
	return os.str();
}

bool AddReplacementEffect::ApplyTo(Environment& env, Runner&) {
	env.replacementEffects.push_back(this->handler);
	return true;
}

std::string AddReplacementEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool RemoveReplacementEffect::ApplyTo(Environment& env, Runner&) {
	std::vector<std::shared_ptr<const EventHandler>>& list = env.replacementEffects;
	list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<const EventHandler> e) ->
		bool { return *e == *this->handler; }), list.end());
	// CodeReview: Return whether it existed in the vector to begin with
	return true;
}

std::string RemoveReplacementEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool AddTriggerEffect::ApplyTo(Environment& env, Runner&) {
	env.triggerHandlers.push_back(this->handler);
	return true;
}

std::string AddTriggerEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool RemoveTriggerEffect::ApplyTo(Environment& env, Runner&) {
	std::vector<std::shared_ptr<const TriggerHandler>>& list = env.triggerHandlers;
	list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<const TriggerHandler> e) ->
		bool { return *e == *this->handler; }), list.end());
	// CodeReview: Return whether it existed in the vector to begin with
	return true;
}

std::string RemoveTriggerEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool AddStaticEffect::ApplyTo(Environment& env, Runner&) {
	// CodeReview: if sqh is a ControlChangeHandler check if the target is not under the new controller's
	// control if so fire control change event for the target
	env.stateQueryHandlers.push_back(this->handler);
	return true;
}

std::string AddStaticEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool RemoveStaticEffect::ApplyTo(Environment& env, Runner&) {
	// CodeReview: if sqh is a ControlChangeHandler get the current controller with that handler
	// Then after removing the handler check again if the controllers are different fire a control change event
	std::vector<std::shared_ptr<const StaticEffectHandler>>& list = env.stateQueryHandlers;
	list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<const StaticEffectHandler> e) ->
		bool { return *e == *this->handler; }), list.end());
	// CodeReview: Return whether it existed in the vector to begin with
	return true;
}

std::string RemoveStaticEffect::ToString() const {
	// CodeReview eventually output something nicer here, maybe class name or toString on replacement effect
	return "";
}

bool AddMana::ApplyTo(Environment& env, Runner&) {
	env.manaPools.at(this->player) += this->amount;
	return true;
}

std::string AddMana::ToString() const {
	std::ostringstream os;
	os << "Add Mana: " << this->player << " gets " << this->amount;
	return os.str();
}

bool RemoveMana::ApplyTo(Environment& env, Runner&) {
	env.manaPools.at(this->player) -= this->amount;
	return true;
}

std::string RemoveMana::ToString() const {
	std::ostringstream os;
	os << "Remove Mana: " << this->player << " uses " << this->amount;
	return os.str();
}

bool DamageToTarget::ApplyTo(Environment& env, Runner& runner) {
	std::shared_ptr<Targetable> pObject = env.gameObjects[this->target];
	if (!pObject) return false;
	if (std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(pObject)) {
		// CodeReview: possible overflow here
		Changeset lifeLoss(std::shared_ptr<GameChange>(new LifeTotalChange{ player->id, (int)this->amount }));
		runner.applyChangeset(lifeLoss);
	}
	else if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(pObject)) {
		env.damage[card->id] += this->amount;
	}
	return true;
}

std::string DamageToTarget::ToString() const {
	std::ostringstream os;
	os << "Damage: " << this->amount << " to " << this->target;
	return os.str();
}

std::string CombatDamageToTarget::ToString() const {
	std::ostringstream os;
	os << "Combat Damage: " << this->amount << " to " << this->target;
	return os.str();
}

bool TapTarget::ApplyTo(Environment& env, Runner&) {
	std::shared_ptr<Targetable> object = env.gameObjects[this->target];
	std::shared_ptr<CardToken> pObject = std::dynamic_pointer_cast<CardToken>(object);
	if (!pObject) return false;
	pObject->isTapped = this->tap;
	return true;
}

std::string TapTarget::ToString() const {
	std::ostringstream os;
	os << (this->tap ? "Tapping: " : "Untapping: ") << this->target;
	return os.str();
}

std::string TapTarget::ToString(Environment& env) const {
	std::shared_ptr<Targetable> object = env.gameObjects[this->target];
	std::shared_ptr<CardToken> pObject = std::dynamic_pointer_cast<CardToken>(object);
	std::ostringstream os;
	if (!pObject) {
		os << "Invalid target for " << (this->tap ? "Tapping " : "Untapping ") << this->target;
		return os.str();
	}
	os << (this->tap ? "Tapping: " : "Untapping: ") << pObject->name << "(" << this->target << ")";
	return os.str();
}

bool CreateTargets::ApplyTo(Environment& env, Runner&) {
	env.targets[this->object] = this->targets;
	return true;
}

std::string CreateTargets::ToString() const {
	std::ostringstream os;
	os << "Creating Targets: " << this->targets << " for " << this->object;
	return os.str();
}

bool StepOrPhaseChange::ApplyTo(Environment& env, Runner& runner) {
	this->starting = env.currentPhase;
	// CodeReview: How to handle extra steps?
	// CodeReview: Handle mana that doesn't empty
	// 500.4. When a step or phase ends, any unused mana left in a player's mana pool empties. This turn-based
	// action doesn't use the stack.
#ifdef DEBUG
	std::cout << "Clearing mana pools" << std::endl;
#endif
	for (auto& manaPool : env.manaPools) {
		manaPool.second.clear();
	}

	// CodeReview: For now have this happen here, not generally safe since multiple cleanup steps can happen
	// Eventually should get moved to happen after moving into an Untap step
	if (env.currentPhase == CLEANUP) {
		// CodeReview: Implement Phasing
		// 502.1. First, all phased-in permanents with phasing that the active player controls phase out, and all
		// phased-out permanents that the active player controlled when they phased out phase in. This all happens
		// simultaneously. This turn-based action doesn't use the stack.
		// CodeReview: Get next player from Environment
		unsigned int nextPlayer = (env.turnPlayer + 1) % env.players.size();
		env.currentPlayer = nextPlayer;
		env.turnPlayer = nextPlayer;
		env.currentPhase = UNTAP;
		env.turnNumber++;
		xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
		env.landPlays[turnPlayerId] = 0;

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
		runner.applyChangeset(untap);
	}
	else if (env.currentPhase == ENDCOMBAT) {
		env.currentPhase = POSTCOMBATMAIN;
		env.declaredAttacks.clear();
		env.declaredBlocks.clear();
		env.blocked.clear();
		env.blockingOrder.clear();
	}
	else {
		env.currentPhase = (StepOrPhase)((int)env.currentPhase + 1);
		env.currentPlayer = env.turnPlayer;
	}
#ifdef DEBUG
	std::cout << "Moving to " << env.currentPhase << std::endl;
#endif

	if (env.currentPhase == DRAW) {
		// 504.1. First, the active player draws a card. This turn-based action doesn't use the stack.
		// CodeReview: Handle first turn don't draw
		Changeset drawCard = Changeset::drawCards(env.players[env.turnPlayer]->id, 1, env);
#ifdef DEBUG
		if (!drawCard.changes.empty()) {
			std::shared_ptr<ObjectMovement> move = std::dynamic_pointer_cast<ObjectMovement>(drawCard.changes[0]);
			std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(move->object));
			std::cout << env.players[env.turnPlayer]->id << "is drawing " << card->name << std::endl;
		}
#endif
		runner.applyChangeset(drawCard);
	}
	else if (env.currentPhase == PRECOMBATMAIN) {
		// CodeReview: If archenemy do a scheme
		// 505.3. First, but only if the players are playing an Archenemy game (see rule 904), the active player is
		// the archenemy, and it's the active player's precombat main phase, the active player sets the top card of
		// their scheme deck in motion (see rule 701.24). This turn-based action doesn't use the stack.
		// CodeReview: Implement Saga's turn based action
		// 505.4. Second, if the active player controls one or more Saga enchantments and it's the active player's
		// precombat main phase, the active player puts a lore counter on each Saga they control. (See rule 714, "Saga Cards.")
		// This turn-based action doesn't use the stack.
	}
	else if (env.currentPhase == BEGINCOMBAT) {
		// 507.1. First, if the game being played is a multiplayer game in which the active player's opponents don't
		// all automatically become defending players, the active player chooses one of their opponents. That player
		// becomes the defending player. This turn-based action doesn't use the stack.
		// CodeReview: Appoint defending. Should always be automatic
	}
	else if (env.currentPhase == DECLAREATTACKERS) {
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
		std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>> declaredAttacks;
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
				declaredAttacks.push_back(attacker.value());
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
	}
	else if (env.currentPhase == DECLAREBLOCKERS) {
		xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
		for (auto& player : env.players) {
			if (player->id == turnPlayerId) continue;
			std::set<xg::Guid> attackers;
			for (auto& attacker : env.declaredAttacks) {
				if (attacker.second == player->id || env.getController(attacker.second) == player->id)
					attackers.insert(attacker.first->id);
			}
			std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>> declaredBlocks;
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
					declaredBlocks.push_back(blocker.value());
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

			env.declaredBlocks.insert(env.declaredBlocks.end(), declaredBlocks.begin(), declaredBlocks.end());
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
	}
	else if (env.currentPhase == FIRSTSTRIKEDAMAGE) {
		// CodeReview: For this phase only consider creatures with first or double strike
		// Copy COMBATDAMAGE code
		// CodeReview: Mark all creatures that dealt damage as having dealt damage this combat
	}
	else if (env.currentPhase == COMBATDAMAGE) {
		Changeset damageEvent;
		if (!env.declaredAttacks.empty()) {
			for (auto& attack : env.declaredAttacks) {
				if (env.blocked.find(attack.first->id) != env.blocked.end()) {
					Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects[env.getController(attack.first)]);
					int powerRemaining = env.getPower(attack.first);
					for (auto& blocker : env.blockingOrder[attack.first->id]) {
						int minDamage = std::min(env.getLethalDamage(attack.first, blocker), powerRemaining);
						unsigned int damageAmount = player.strategy->chooseDamageAmount(attack.first, blocker, minDamage, powerRemaining, env);
						powerRemaining -= damageAmount;
						damageEvent.changes.push_back(std::shared_ptr<GameChange>(new CombatDamageToTarget{ blocker, damageAmount, attack.first->id }));
						damageEvent.changes.push_back(std::shared_ptr<GameChange>(new CombatDamageToTarget{ attack.first->id, (unsigned int)std::max(env.getPower(blocker), 0), blocker }));
					}
				}
				else {
					damageEvent.changes.push_back(std::shared_ptr<GameChange>(new DamageToTarget{ attack.second, (unsigned int)std::max(env.getPower(attack.first), 0), attack.first->id }));
				}
			}
#ifdef DEBUG
			std::cout << "Applying combat damage" << std::endl;
#endif
			runner.applyChangeset(damageEvent);
		}
		// CodeReview:For this phase do not consider creatures with first strike that dealt damage already this combat
		// CodeReview: If multiple attackers(similar for blockers) would damage the same creature the total has to be lethal to continue in blocking order but not the individual parcels
	}
	else if (env.currentPhase == CLEANUP) {
		xg::Guid turnPlayerId = env.players[env.turnPlayer]->id;
		env.currentPhase = CLEANUP;
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
	}
	return true;
}

std::string StepOrPhaseChange::ToString() const {
	std::ostringstream os;
	os << "Step or Phase Change: leaving " << this->starting;
	return os.str();
}

bool ClearTriggers::ApplyTo(Environment& env, Runner&) {
	env.triggers.clear();
	return true;
}

std::string ClearTriggers::ToString() const {
	return "Clearing Triggers";
}

bool QueueTrigger::ApplyTo(Environment& env, Runner&) {
	env.triggers.push_back(*this);
	return true;
}

std::string QueueTrigger::ToString() const {
	std::ostringstream os;
	os << "Queue Trigger: " << this->ability->id << " for source " << this->source << " controlled by player " << this->player
		<< "triggered by" << this->triggered;
	return os.str();
}

std::string QueueTrigger::ToString(Environment& env) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(this->source));
	std::ostringstream os;
	os << "Queue Trigger: " << this->ability->id << " for source " << card->name << " controlled by player " << this->player
		<< "triggered by" << this->triggered;
	return os.str();
}

bool LandPlay::ApplyTo(Environment& env, Runner& runner) {
	env.landPlays[this->player] += 1;
	Changeset moveLand;
	moveLand.changes.push_back(std::shared_ptr<GameChange>(new ObjectMovement{ this->land, this->zone, env.battlefield->id }));
	return runner.applyChangeset(moveLand);
}

std::string LandPlay::ToString() const {
	std::ostringstream os;
	os << "Playing Land: " << this->land << " from " << this->zone << " by " << this->player;
	return os.str();
}

std::string LandPlay::ToString(Environment& env) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(this->land));
	std::ostringstream os;
	os << "Playing Land: " << card->name << " from " << this->zone << " by " << this->player << " whose played " << env.landPlays[this->player] << " lands this turn";
	return os.str();
}

bool ApplyManaAbility::ApplyTo(Environment& env, Runner& runner) {
	Changeset apply = this->manaAbility->applyEffect(env);
	return runner.applyChangeset(apply);
}

std::string ApplyManaAbility::ToString() const {
	std::ostringstream os;
	os << "Applying Mana Ability: " << this->manaAbility->id << " of " << getBaseClassPtr<const CardToken>(this->manaAbility->source)->name;
	return os.str();
}

bool LoseTheGame::ApplyTo(Environment& env, Runner& runner) {
	unsigned int index = 0;
	for (auto iter = env.players.begin(); iter != env.players.end(); iter++) {
		if ((*iter)->id == this->player) {
			if (env.turnPlayer == index)
			{
				// CodeReview: Exile everything on the stack
				env.currentPhase = END;
				Changeset endTurn;
				endTurn.changes.push_back(std::shared_ptr<GameChange>(new StepOrPhaseChange{ END }));
				runner.applyChangeset(endTurn);
			}
			if (env.currentPlayer == index) env.currentPlayer = (env.currentPlayer + 1) % env.players.size();
			// CodeReview remove unneeded data structures
			// CodeReview remove eventhandlers registered to that player
			// Need a way to find what zone an object is in to do this for all objects
			// This isn't working currently for an unknown reason
			Changeset removeCards;
			for (auto& card : env.battlefield->objects) {
				if (getBaseClassPtr<const Targetable>(card)->owner == this->player) {
					removeCards.changes.push_back(std::shared_ptr<GameChange>(new RemoveObject{ getBaseClassPtr<const Targetable>(card)->id, env.battlefield->id }));
				}
			}
			runner.applyChangeset(removeCards, false, false);
			env.players.erase(iter);
			break;
		}
		index++;
	}
	return true;
}

std::string LoseTheGame::ToString() const {
	std::ostringstream os;
	os << "Lost the Game: " << this->player;
	return os.str();
}

bool DeclareAttack::ApplyTo(Environment&, Runner&) {
	return true;
}

std::string DeclareAttack::ToString() const {
	std::ostringstream os;
	os << "Declare Attack: " << this->attacker << " attacking " << this->defender;
	return os.str();
}

std::string DeclareAttack::ToString(Environment& env) const {
	std::shared_ptr<CardToken> attackerCard = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(this->attacker));
	std::shared_ptr<CardToken> defenderCard = std::dynamic_pointer_cast<CardToken>(env.gameObjects.at(this->defender));
	std::ostringstream os;
	os << "Declare Attack: " << attackerCard->name << " attacking ";
	if (defenderCard) os << defenderCard->name;
	else os << this->defender;
	return os.str();
}

Changeset Changeset::operator+(const Changeset& other) const {
	std::vector<std::shared_ptr<GameChange>> newChanges{ this->changes };
	newChanges.insert(newChanges.end(), other.changes.begin(), other.changes.end());
	return Changeset{ newChanges };
}

Changeset& Changeset::operator+=(const Changeset& other) {
	this->changes.insert(this->changes.end(), other.changes.begin(), other.changes.end());
	return *this;
}

bool Changeset::empty() const {
	return this->changes.empty();
}

std::ostream& operator<<(std::ostream& os, const Changeset& changeset) {
	os << "Beginning Changeset" << std::endl;

	for (std::shared_ptr<GameChange> change : changeset.changes) {
		os << change->ToString() << std::endl;
	}

	os << "Ending Changeset" << std::endl << std::endl;
	return os;
}

Changeset Changeset::drawCards(xg::Guid player, size_t amount, const Environment& env) {
	Changeset result;
	const Zone<Card, Token>& libraryZone = *env.libraries.at(player);
	auto library = libraryZone.objects;
	const Zone<Card, Token>& handZone = *env.hands.at(player);
	if (amount > library.size()) {
		// CodeReview: Should wait to lose until next time state based actions are checked
		result.changes.push_back(std::shared_ptr<GameChange>(new LoseTheGame{ player }));
		amount = library.size();
	}
	auto card = library.begin() + (library.size() - amount);
	for (; card != library.end(); card++) {
		std::shared_ptr<const Targetable> c = getBaseClassPtr<const Targetable>(*card);
		result.changes.push_back(std::shared_ptr<GameChange>(new ObjectMovement{ c->id, libraryZone.id, handZone.id, 0, DRAWCARD }));
	}
	return result;
}

Changeset Changeset::discardCards(xg::Guid playerId, size_t amount, const Environment& env) {
	Changeset result;
	const Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects.at(playerId));
	std::vector<xg::Guid> cards = player.strategy->chooseDiscards(amount, player, env);
	for (xg::Guid& guid : cards) result.changes.push_back(std::shared_ptr<GameChange>(new ObjectMovement{ guid, env.hands.at(playerId)->id, env.graveyards.at(playerId)->id, 0, DISCARD }));
	return result;
}

template<typename T>
std::vector<std::pair<std::vector<T>, std::vector<T>>> createCombinations(std::vector<T> vec) {
	std::vector<std::pair<std::vector<T>, std::vector<T>>> result;
	T t = vec.back();
	if (vec.size() == 1) return { { { t },{} },{ {},{ t } } };
	vec.erase(vec.end() - 1);
	result = createCombinations(vec);
	size_t curSize = result.size();
	result.insert(result.begin(), result.begin(), result.end());
	for (size_t i = 0; i < curSize; i++) result[i].first.push_back(t);
	for (size_t i = curSize; i < result.size(); i++) result[i].second.push_back(t);
	return result;
}

template<typename T>
std::vector<std::vector<T>> createPermutations(std::vector<T> vec) {
	if (vec.empty()) return { {} };
	std::vector<std::vector<T>> result;
	for (int i = 0; i < vec.size(); i++) {
		std::vector<T> vec2 = vec;
		T t = vec2[i];
		vec2.erase(vec2.begin() + i);
		std::vector<std::vector<T>> remaining = createPermutations(vec2);
		for (auto& v : remaining) {
			v.push_back(t);
		}
		result.insert(result.end(), remaining.begin(), remaining.end());
	}
	return result;
}

Changeset Changeset::scryCards(xg::Guid player, size_t amount, const Environment& env) {
	Changeset result;
	const Zone<Card, Token>& libraryZone = *env.libraries.at(player);
	auto library = libraryZone.objects;
	if (amount > library.size()) {
		amount = library.size();
	}
	std::vector<xg::Guid> cards;
	cards.reserve(amount);
	auto card = library.begin() + (library.size() - amount);
	for (; card != library.end(); card++) {
		std::shared_ptr<const Targetable> c = getBaseClassPtr<const Targetable>(*card);
		cards.push_back(c->id);
	}
	std::vector<Changeset> changes;
	std::vector<std::vector<xg::Guid>> allPermutations = createPermutations(cards);
	std::vector<std::pair<std::vector<xg::Guid>, std::vector<xg::Guid>>> options;
	for (auto& p : allPermutations) {
		std::vector<std::pair<std::vector<xg::Guid>, std::vector<xg::Guid>>> comb = createCombinations(p);
		options.insert(options.end(), comb.begin(), comb.end());
	}
	for (auto& p : options) {
		Changeset change;
		for (auto t : p.first) {
			change.changes.push_back(std::shared_ptr<GameChange>(new ObjectMovement{ t, libraryZone.id, libraryZone.id }));
		}
		for (auto b : p.second) {
			change.changes.push_back(std::shared_ptr<GameChange>(new ObjectMovement{ b, libraryZone.id, libraryZone.id, -1 }));
		}
		changes.push_back(change);
	}
	std::shared_ptr<Player> p = std::dynamic_pointer_cast<Player>(env.gameObjects.at(player));
	return p->strategy->chooseOne(changes, *p, env);
}