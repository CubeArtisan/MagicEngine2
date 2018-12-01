#include <iostream>
#include <sstream>
#include <string>

#include "ability.h"
#include "changeset.h"
#include "environment.h"
#include "runner.h"
#include "stepOrPhase.h"

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

StepOrPhaseId StepOrPhaseChange::nextStepOrPhase(Environment& env) const {
	// CodeReview: How to handle extra steps/phases?
	if (env.currentPhase != this->starting) return STEPORPHASECOUNT;
	return (StepOrPhaseId)((int)env.currentPhase + 1);
}

bool StepOrPhaseChange::ApplyTo(Environment& env, Runner& runner) {
	this->starting = env.currentPhase;

	bool result = true;
	std::shared_ptr<const StepOrPhase> currentPhase = StepOrPhase::getStepOrPhase(env.currentPhase);
	result &= currentPhase->applyLeave(env, runner);
	StepOrPhaseId next = this->nextStepOrPhase(env);
	if (next == STEPORPHASECOUNT) return true;
#ifdef DEBUG
	std::cout << "Moving to " << next << std::endl;
#endif
	// CodeReview: Handle mana that doesn't empty
	// 500.4. When a step or phase ends, any unused mana left in a player's mana pool empties. This turn-based
	// action doesn't use the stack.
#ifdef DEBUG
	std::cout << "Clearing mana pools" << std::endl;
#endif
	for (auto& manaPool : env.manaPools) {
		manaPool.second.clear();
	}
	env.currentPhase = next;
	env.currentPlayer = env.turnPlayer;
	std::shared_ptr<const StepOrPhase> nextPhase = StepOrPhase::getStepOrPhase(next);
	result &= nextPhase->applyEnter(env, runner);
	return result;
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