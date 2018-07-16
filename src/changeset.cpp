#include "changeset.h"
#include "environment.h"

Targetable::Targetable()
    : id(xg::newGuid())
{}

Changeset Changeset::operator+(Changeset& other){
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
    std::vector<std::shared_ptr<EventHandler>> eventsToAdd = this->eventsToAdd;
    eventsToAdd.insert(eventsToAdd.end(), other.eventsToAdd.begin(), other.eventsToAdd.end());
    std::vector<std::shared_ptr<EventHandler>> eventsToRemove = this->eventsToRemove;
    eventsToRemove.insert(eventsToRemove.end(), other.eventsToRemove.begin(), other.eventsToRemove.end());
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToAdd = this->propertiesToAdd;
    propertiesToAdd.insert(propertiesToAdd.end(), other.propertiesToAdd.begin(), other.propertiesToAdd.end());
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToRemove = this->propertiesToRemove;
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
    StepOrPhaseChange phaseChange = this->phaseChange;
    if(!phaseChange.changed && other.phaseChange.changed){
        phaseChange = other.phaseChange;
    }

    return Changeset{moves, playerCounters, permanentCounters, create, remove, lifeTotalChanges, eventsToAdd,
                     eventsToRemove, propertiesToAdd, propertiesToRemove, loseTheGame, addMana, removeMana, damage, tap,
                     phaseChange};
}

Changeset& Changeset::operator+=(Changeset other){
    moves.insert(moves.end(), other.moves.begin(), other.moves.end());
    playerCounters.insert(playerCounters.end(), other.playerCounters.begin(), other.playerCounters.end());
    permanentCounters.insert(permanentCounters.end(), other.permanentCounters.begin(), other.permanentCounters.end());
    create.insert(create.end(), other.create.begin(), other.create.end());
    remove.insert(remove.end(), other.remove.begin(), other.remove.end());
    lifeTotalChanges.insert(lifeTotalChanges.end(), other.lifeTotalChanges.begin(), other.lifeTotalChanges.end());
    eventsToAdd.insert(eventsToAdd.end(), other.eventsToAdd.begin(), other.eventsToAdd.end());
    eventsToRemove.insert(eventsToRemove.end(), other.eventsToRemove.begin(), other.eventsToRemove.end());
    propertiesToAdd.insert(propertiesToAdd.end(), other.propertiesToAdd.begin(), other.propertiesToAdd.end());
    propertiesToRemove.insert(propertiesToRemove.end(), other.propertiesToRemove.begin(), other.propertiesToRemove.end());
    loseTheGame.insert(loseTheGame.end(), other.loseTheGame.begin(), other.loseTheGame.end());
    addMana.insert(addMana.end(), other.addMana.begin(), other.addMana.end());
    removeMana.insert(removeMana.end(), other.removeMana.begin(), other.removeMana.end());
    damage.insert(damage.end(), other.damage.begin(), other.damage.end());
    tap.insert(tap.end(), other.tap.begin(), other.tap.end());
    if(!phaseChange.changed && other.phaseChange.changed){
        phaseChange = other.phaseChange;
    }

    return *this;
}

std::ostream& operator<<(std::ostream& os, Changeset& changeset) {
	os << "Beginning Changeset" << std::endl;
    if(changeset.moves.size() > 0) {
        for(ObjectMovement& move : changeset.moves) {
            os << "Movement: " << move.object << " from " << move.sourceZone << " to " << move.destinationZone
               << " new GUID " << move.newObject << std::endl;
        }
    }
    if(changeset.addMana.size() > 0) {
        for(AddMana& mana : changeset.addMana) {
            os << "Add Mana: " << mana.player << " gets " << mana.amount << std::endl;
        }
    }
    if(changeset.removeMana.size() > 0) {
        for(RemoveMana& mana : changeset.removeMana) {
            os << "Remove Mana: " << mana.player << " gets " << mana.amount << std::endl;
        }
    }
    if(changeset.tap.size() > 0) {
        for(TapTarget& tap : changeset.tap) {
            os << "Tapping Target: " << tap.target << " " << tap.tap << std::endl;
        }
    }
    if(changeset.phaseChange.changed) {
        os << "Leaving step " << changeset.phaseChange.starting << std::endl;
    }
	os << "Ending Changeset" << std::endl << std::endl;
    return os;
}

Changeset Changeset::drawCards(xg::Guid player, size_t amount, const Environment& env){
    Changeset result = Changeset();
    const Zone<Card, Token>& libraryZone = env.libraries.at(player);
    auto library = libraryZone.objects;
    const Zone<Card, Token>& handZone = env.hands.at(player);
    if(amount > library.size()){
		result.loseTheGame.push_back(player);
        amount = library.size();
    }
    auto card = library.begin() + (library.size() - amount);
    for(; card != library.end(); card++) {
        std::shared_ptr<Targetable> c = getBaseClassPtr<Targetable>(*card);
        result.moves.push_back(ObjectMovement{c->id, libraryZone.id, handZone.id});
    }
    return result;
}
