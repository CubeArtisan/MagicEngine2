#include "ability.h"
#include "changeset.h"
#include "environment.h"

Targetable::Targetable()
    : id(xg::newGuid())
{}

std::optional<std::vector<Changeset>> TriggerHandler::handleEvent(Changeset& changeset, const Environment& env) const {
	std::vector<Changeset> result;
	std::vector<QueueTrigger> queue = this->createTriggers(changeset, env);
	if (queue.empty()) return std::nullopt;
	Changeset createTrigger;
	for(QueueTrigger& q : queue) createTrigger.trigger.push_back(q);
	result.push_back(createTrigger);
	return result;
}

Changeset Changeset::operator+(const Changeset& other) const {
    std::vector<ObjectMovement> moves2 = this->moves;
    moves2.insert(moves2.end(), other.moves.begin(), other.moves.end());
    std::vector<AddPlayerCounter> playerCounters2 = this->playerCounters;
    playerCounters2.insert(playerCounters2.end(), other.playerCounters.begin(), other.playerCounters.end());
    std::vector<AddPermanentCounter> permanentCounters2 = this->permanentCounters;
    permanentCounters2.insert(permanentCounters2.end(), other.permanentCounters.begin(), other.permanentCounters.end());
    std::vector<ObjectCreation> create2 = this->create;
    create2.insert(create2.end(), other.create.begin(), other.create.end());
    std::vector<RemoveObject> remove2 = this->remove;
    remove2.insert(remove2.end(), other.remove.begin(), other.remove.end());
    std::vector<LifeTotalChange> lifeTotalChanges2 = this->lifeTotalChanges;
    lifeTotalChanges2.insert(lifeTotalChanges2.end(), other.lifeTotalChanges.begin(), other.lifeTotalChanges.end());
	std::vector<std::shared_ptr<const EventHandler>> effectsToAdd2 = this->effectsToAdd;
	effectsToAdd2.insert(effectsToAdd2.end(), other.effectsToAdd.begin(), other.effectsToAdd.end());
	std::vector<std::shared_ptr<const EventHandler>> effectsToRemove2 = this->effectsToRemove;
	effectsToRemove2.insert(effectsToRemove2.end(), other.effectsToRemove.begin(), other.effectsToRemove.end());
	std::vector<std::shared_ptr<const TriggerHandler>> triggersToAdd2 = this->triggersToAdd;
    triggersToAdd2.insert(triggersToAdd2.end(), other.triggersToAdd.begin(), other.triggersToAdd.end());
    std::vector<std::shared_ptr<const TriggerHandler>> triggersToRemove2 = this->triggersToRemove;
    triggersToRemove2.insert(triggersToRemove2.end(), other.triggersToRemove.begin(), other.triggersToRemove.end());
	std::vector<std::shared_ptr<const StaticEffectHandler>> propertiesToAdd2 = this->propertiesToAdd;
    propertiesToAdd2.insert(propertiesToAdd2.end(), other.propertiesToAdd.begin(), other.propertiesToAdd.end());
    std::vector<std::shared_ptr<const StaticEffectHandler>> propertiesToRemove2 = this->propertiesToRemove;
    propertiesToRemove2.insert(propertiesToRemove2.end(), other.propertiesToRemove.begin(), other.propertiesToRemove.end());
    std::vector<xg::Guid> loseTheGame2 = this->loseTheGame;
    loseTheGame2.insert(loseTheGame2.end(), other.loseTheGame.begin(), other.loseTheGame.end());
    std::vector<AddMana> addMana2 = this->addMana;
    addMana2.insert(addMana2.end(), other.addMana.begin(), other.addMana.end());
    std::vector<RemoveMana> removeMana2 = this->removeMana;
    removeMana2.insert(removeMana2.end(), other.removeMana.begin(), other.removeMana.end());
    std::vector<DamageToTarget> damage2 = this->damage;
    damage2.insert(damage2.end(), other.damage.begin(), other.damage.end());
	std::vector<DamageToTarget> combatDamage2 = this->combatDamage;
	combatDamage2.insert(combatDamage2.end(), other.combatDamage.begin(), other.combatDamage.end());
    std::vector<TapTarget> tap2 = this->tap;
    tap2.insert(tap2.end(), other.tap.begin(), other.tap.end());
	std::vector<CreateTargets> target2 = this->target;
	target2.insert(target2.end(), other.target.begin(), other.target.end());
	std::vector<QueueTrigger> trigger2 = this->trigger;
	trigger2.insert(trigger2.end(), other.trigger.begin(), other.trigger.end());
	std::vector<LandPlay> land2 = this->land;
	land2.insert(land2.end(), other.land.begin(), other.land.end());
	std::vector<std::shared_ptr<ManaAbility>> manaAbility2 = this->manaAbility;
	manaAbility2.insert(manaAbility2.end(), other.manaAbility.begin(), other.manaAbility.end());
	std::vector<DeclareAttack> attacks2 = this->attacks;
	attacks2.insert(attacks2.end(), other.attacks.begin(), other.attacks.end());
    StepOrPhaseChange phaseChange2 = this->phaseChange;
    if(!phaseChange2.changed && other.phaseChange.changed){
        phaseChange2 = other.phaseChange;
    }
	bool clearTriggers2 = this->clearTriggers || other.clearTriggers;

    return Changeset{moves2, playerCounters2, permanentCounters2, create2, remove2, lifeTotalChanges2, effectsToAdd2, effectsToRemove2,
					 triggersToAdd2, triggersToRemove2, propertiesToAdd2, propertiesToRemove2, loseTheGame2, addMana2, removeMana2, damage2,
					 combatDamage2, tap2, target2, trigger2, land2, manaAbility2, attacks2, phaseChange2, clearTriggers2};
}

Changeset& Changeset::operator+=(const Changeset& other){
    moves.insert(moves.end(), other.moves.begin(), other.moves.end());
    playerCounters.insert(playerCounters.end(), other.playerCounters.begin(), other.playerCounters.end());
    permanentCounters.insert(permanentCounters.end(), other.permanentCounters.begin(), other.permanentCounters.end());
    create.insert(create.end(), other.create.begin(), other.create.end());
    remove.insert(remove.end(), other.remove.begin(), other.remove.end());
    lifeTotalChanges.insert(lifeTotalChanges.end(), other.lifeTotalChanges.begin(), other.lifeTotalChanges.end());
	effectsToAdd.insert(effectsToAdd.end(), other.effectsToAdd.begin(), other.effectsToAdd.end());
	effectsToRemove.insert(effectsToRemove.end(), other.effectsToRemove.begin(), other.effectsToRemove.end());
	triggersToAdd.insert(triggersToAdd.end(), other.triggersToAdd.begin(), other.triggersToAdd.end());
    triggersToRemove.insert(triggersToRemove.end(), other.triggersToRemove.begin(), other.triggersToRemove.end());
    propertiesToAdd.insert(propertiesToAdd.end(), other.propertiesToAdd.begin(), other.propertiesToAdd.end());
    propertiesToRemove.insert(propertiesToRemove.end(), other.propertiesToRemove.begin(), other.propertiesToRemove.end());
    loseTheGame.insert(loseTheGame.end(), other.loseTheGame.begin(), other.loseTheGame.end());
    addMana.insert(addMana.end(), other.addMana.begin(), other.addMana.end());
    removeMana.insert(removeMana.end(), other.removeMana.begin(), other.removeMana.end());
    damage.insert(damage.end(), other.damage.begin(), other.damage.end());
    tap.insert(tap.end(), other.tap.begin(), other.tap.end());
	target.insert(target.end(), other.target.begin(), other.target.end());
	trigger.insert(trigger.end(), other.trigger.begin(), other.trigger.end());
	land.insert(land.end(), other.land.begin(), other.land.end());
	manaAbility.insert(manaAbility.end(), other.manaAbility.begin(), other.manaAbility.end());
	attacks.insert(attacks.end(), other.attacks.begin(), other.attacks.end());
	if(!phaseChange.changed && other.phaseChange.changed){
        phaseChange = other.phaseChange;
    }
	this->clearTriggers |= other.clearTriggers;

    return *this;
}

bool Changeset::empty() const {
	return moves.empty() && playerCounters.empty() && permanentCounters.empty() && create.empty() && remove.empty()
			&& lifeTotalChanges.empty() && effectsToAdd.empty() && effectsToRemove.empty() && triggersToAdd.empty()
			&& triggersToRemove.empty() && propertiesToAdd.empty() && propertiesToRemove.empty() && loseTheGame.empty()
			&& addMana.empty() && removeMana.empty() && damage.empty() && combatDamage.empty() && tap.empty()
			&& target.empty() && trigger.empty() && land.empty() && manaAbility.empty() && attacks.empty() && !phaseChange.changed
			&& !clearTriggers;
}

std::ostream& operator<<(std::ostream& os, const Changeset& changeset) {
	os << "Beginning Changeset" << std::endl;

    for(const AddMana& mana : changeset.addMana) {
        os << "Add Mana: " << mana.player << " gets " << mana.amount << std::endl;
    }
    for(const RemoveMana& mana : changeset.removeMana) {
        os << "Remove Mana: " << mana.player << " uses " << mana.amount << std::endl;
    }
    for(const TapTarget& tap : changeset.tap) {
		if(tap.tap) os << "Tapping Target: " << tap.target << std::endl;
		else os << "Untapping Target: " << tap.target << std::endl;
    }
	for (const ObjectCreation& create : changeset.create) {
		os << "Creating object: " << create.created->id << " in " << create.zone << std::endl;
	}
	for (const RemoveObject& remove : changeset.remove) {
		os << "Removing object: " << remove.object << " from " << remove.zone << std::endl;
	}
	for (const CreateTargets& target : changeset.target) {
		os << "Creating targets for " << target.object << " with targets";
		for(const xg::Guid& t : target.targets) {
			os << ' ' << t << std::endl;
		}
	}
	for (const LifeTotalChange& lifeTotalChange : changeset.lifeTotalChanges) {
		os << lifeTotalChange.player << " life total changing from " << lifeTotalChange.oldValue
		   << " to " << lifeTotalChange.newValue << std::endl;
	}
	for (const DamageToTarget& damage : changeset.damage) {
		os << damage.amount << " damage to  " << damage.target << " from " << damage.source << std::endl;
	}
	for (const DamageToTarget& damage : changeset.combatDamage) {
		os << damage.amount << " combat damage to  " << damage.target << " from " << damage.source << std::endl;
	}
	if (changeset.phaseChange.changed) {
		os << "Leaving step " << changeset.phaseChange.starting << std::endl;
	}
	for (const LandPlay& land : changeset.land) {
		os << "Playing a land: " << land.land << " by " << land.player << " from " << land.zone << std::endl;
	}
	for (const DeclareAttack& attack : changeset.attacks) {
		os << attack.attacker << " is attacking " << attack.defender << std::endl;
	}
	for (const std::shared_ptr<ManaAbility>& manaAbility : changeset.manaAbility) {
		os << "Activating a mana ability: " << manaAbility->id << std::endl;
	}
	for (const ObjectMovement& move : changeset.moves) {
		os << "Movement: " << move.object << " from " << move.sourceZone << " to " << move.destinationZone
			<< " new GUID " << move.newObject << std::endl;
	}
	for (const xg::Guid& player : changeset.loseTheGame) {
		os << player << " loses the game" << std::endl;
	}
	os << "Ending Changeset" << std::endl << std::endl;
    return os;
}

Changeset Changeset::drawCards(xg::Guid player, size_t amount, const Environment& env){
    Changeset result;
    const Zone<Card, Token>& libraryZone = *env.libraries.at(player);
    auto library = libraryZone.objects;
    const Zone<Card, Token>& handZone = *env.hands.at(player);
    if(amount > library.size()){
		// CodeReview: Should wait to lose until next time state based actions are checked
		result.loseTheGame.push_back(player);
        amount = library.size();
    }
    auto card = library.begin() + (library.size() - amount);
    for(; card != library.end(); card++) {
        std::shared_ptr<const Targetable> c = getBaseClassPtr<const Targetable>(*card);
        result.moves.push_back(ObjectMovement{c->id, libraryZone.id, handZone.id, 0, DRAWCARD});
    }
    return result;
}


Changeset Changeset::discardCards(xg::Guid playerId, size_t amount, const Environment& env) {
	Changeset result;
	const Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects.at(playerId));
	std::vector<xg::Guid> cards = player.strategy->chooseDiscards(amount, player, env);
	for (xg::Guid& guid : cards) result.moves.push_back(ObjectMovement{ guid, env.hands.at(playerId)->id, env.graveyards.at(playerId)->id, 0, DISCARD });
	return result;
}

template<typename T>
std::vector<std::pair<std::vector<T>, std::vector<T>>> createCombinations(std::vector<T> vec) {
	std::vector<std::pair<std::vector<T>, std::vector<T>>> result;
	T t = vec.back();
	if (vec.size() == 1) return { { { t },{} },{ {},{ t } } };
	vec.erase(vec.end()-1);
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
			change.moves.push_back(ObjectMovement{ t, libraryZone.id, libraryZone.id });
		}
		for (auto b : p.second) {
			change.moves.push_back(ObjectMovement{ b, libraryZone.id, libraryZone.id, -1 });
		}
		changes.push_back(change);
	}
	std::shared_ptr<Player> p = std::dynamic_pointer_cast<Player>(env.gameObjects.at(player));
	return p->strategy->chooseOne(changes, *p, env);
}