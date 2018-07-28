#include "changeset.h"
#include "environment.h"

Targetable::Targetable()
    : id(xg::newGuid())
{}

std::variant<std::vector<Changeset>, PassPriority> TriggerHandler::handleEvent(Changeset& changeset, const Environment& env) const {
	std::vector<Changeset> result;
	std::vector<QueueTrigger> queue = this->createTriggers(changeset, env);
	if (queue.empty()) return PassPriority();
	Changeset createTrigger;
	for(QueueTrigger& q : queue) createTrigger.trigger.push_back(q);
	result.push_back(createTrigger);
	return result;
}

Changeset Changeset::operator+(const Changeset& other){
    std::vector<ObjectMovement> moves = this->moves;
    moves.insert(moves.end(), other.moves.begin(), other.moves.end());
    std::vector<AddPlayerCounter> playerCounters = this->playerCounters;
    playerCounters.insert(playerCounters.end(), other.playerCounters.begin(), other.playerCounters.end());
    std::vector<AddPermanentCounter> permanentCounters = this->permanentCounters;
    permanentCounters.insert(permanentCounters.end(), other.permanentCounters.begin(), other.permanentCounters.end());
    std::vector<ObjectCreation> create = this->create;
    create.insert(create.end(), other.create.begin(), other.create.end());
    std::vector<RemoveObject> remove = this->remove;
    remove.insert(remove.end(), other.remove.begin(), other.remove.end());
    std::vector<LifeTotalChange> lifeTotalChanges = this->lifeTotalChanges;
    lifeTotalChanges.insert(lifeTotalChanges.end(), other.lifeTotalChanges.begin(), other.lifeTotalChanges.end());
	std::vector<std::shared_ptr<EventHandler>> effectsToAdd = this->effectsToAdd;
	effectsToAdd.insert(effectsToAdd.end(), other.effectsToAdd.begin(), other.effectsToAdd.end());
	std::vector<std::shared_ptr<EventHandler>> effectsToRemove = this->effectsToRemove;
	effectsToRemove.insert(effectsToRemove.end(), other.effectsToRemove.begin(), other.effectsToRemove.end());
	std::vector<std::shared_ptr<TriggerHandler>> triggersToAdd = this->triggersToAdd;
    triggersToAdd.insert(triggersToAdd.end(), other.triggersToAdd.begin(), other.triggersToAdd.end());
    std::vector<std::shared_ptr<TriggerHandler>> triggersToRemove = this->triggersToRemove;
    triggersToRemove.insert(triggersToRemove.end(), other.triggersToRemove.begin(), other.triggersToRemove.end());
	std::vector<std::shared_ptr<StaticEffectHandler>> propertiesToAdd = this->propertiesToAdd;
    propertiesToAdd.insert(propertiesToAdd.end(), other.propertiesToAdd.begin(), other.propertiesToAdd.end());
    std::vector<std::shared_ptr<StaticEffectHandler>> propertiesToRemove = this->propertiesToRemove;
    propertiesToRemove.insert(propertiesToRemove.end(), other.propertiesToRemove.begin(), other.propertiesToRemove.end());
    std::vector<xg::Guid> loseTheGame = this->loseTheGame;
    loseTheGame.insert(loseTheGame.end(), other.loseTheGame.begin(), other.loseTheGame.end());
    std::vector<AddMana> addMana = this->addMana;
    addMana.insert(addMana.end(), other.addMana.begin(), other.addMana.end());
    std::vector<RemoveMana> removeMana = this->removeMana;
    removeMana.insert(removeMana.end(), other.removeMana.begin(), other.removeMana.end());
    std::vector<DamageToTarget> damage = this->damage;
    damage.insert(damage.end(), other.damage.begin(), other.damage.end());
    std::vector<TapTarget> tap = this->tap;
    tap.insert(tap.end(), other.tap.begin(), other.tap.end());
	std::vector<CreateTargets> target = this->target;
	target.insert(target.end(), other.target.begin(), other.target.end());
	std::vector<QueueTrigger> trigger = this->trigger;
	trigger.insert(trigger.end(), other.trigger.begin(), other.trigger.end());
	std::vector<LandPlay> land = this->land;
	land.insert(land.end(), other.land.begin(), other.land.end());
	std::vector<std::shared_ptr<ManaAbility>> manaAbility = this->manaAbility;
	manaAbility.insert(manaAbility.end(), other.manaAbility.begin(), other.manaAbility.end());
	std::vector<DeclareAttack> attacks = this->attacks;
	attacks.insert(attacks.end(), other.attacks.begin(), other.attacks.end());
    StepOrPhaseChange phaseChange = this->phaseChange;
    if(!phaseChange.changed && other.phaseChange.changed){
        phaseChange = other.phaseChange;
    }
	bool clearTriggers = this->clearTriggers || other.clearTriggers;

    return Changeset{moves, playerCounters, permanentCounters, create, remove, lifeTotalChanges, effectsToAdd, effectsToRemove,
					 triggersToAdd, triggersToRemove, propertiesToAdd, propertiesToRemove, loseTheGame, addMana, removeMana, damage, tap,
                     target, trigger, land, manaAbility, attacks, phaseChange, clearTriggers};
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
		os << damage.amount << " damage to  " << damage.target << std::endl;
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
        result.moves.push_back(ObjectMovement{c->id, libraryZone.id, handZone.id});
    }
    return result;
}


Changeset Changeset::discardCards(xg::Guid playerId, size_t amount, const Environment& env) {
	Changeset result;
	const Player& player = *std::dynamic_pointer_cast<Player>(env.gameObjects.at(playerId));
	std::vector<xg::Guid> cards = player.strategy->chooseDiscards(amount, player, env);
	for (xg::Guid& guid : cards) result.moves.push_back(ObjectMovement{ guid, env.hands.at(playerId)->id, env.graveyards.at(playerId)->id });
	return result;
}