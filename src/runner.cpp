#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"

std::variant<Changeset, PassPriority> Runner::executeStep()
{
    Player& active = (Player&)this->env.players[this->env.currentPlayer];
    GameAction action = active.strategy.chooseGameAction(active, env);

    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        Changeset castSpell;
        Targetable hand = this->env.hands[active.id];
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand.id, this->env.stack.id});
        // CodeReview: Assign targets
        castSpell += pCastSpell->cost.payCost(active, env);
        for(Cost& c : pCastSpell->additionalCosts) {
            castSpell += c.payCost(active, env);
        }
        // CodeReview: Use the chosen X value
        return castSpell;
    }

    if(PlayLand* pPlayLand = std::get_if<PlayLand>(&action)){
        std::vector<Changeset> results;
        Changeset playLand;
        Targetable hand = this->env.hands[active.id];
        playLand.moves.push_back(ObjectMovement{pPlayLand->land, hand.id, this->env.stack.id});
        // CodeReview: Use land play for the turn
        return playLand;
    }

    if(ActivateAnAbility* pActivateAnAbility = std::get_if<ActivateAnAbility>(&action)){
        Changeset activateAbility;
        ActivatedAbility& result = pActivateAnAbility->ability;
        result.source = pActivateAnAbility->source;
        activateAbility.create.push_back(ObjectCreation{this->env.stack.id, result});
        // CodeReview: Assign targets
        activateAbility += pActivateAnAbility->cost.payCost(active, env);
        // CodeReview: Use the chosen X value
        return activateAbility;
    }

    if(std::get_if<PassPriority>(&action)){
        return PassPriority();
    }

    return Changeset();
}

void Runner::runGame(){
    int firstPlayerToPass = -1;
    while(this->env.players.size() > 0) {
        std::variant<Changeset, PassPriority> step = this->executeStep();

        if(Changeset* pChangeset = std::get_if<Changeset>(&step)){
            this->applyChangeset(*pChangeset);
            firstPlayerToPass = -1;
        }
        else {
            int nextPlayer = (this->env.currentPlayer + 1) % this->env.players.size();
            if(firstPlayerToPass == nextPlayer){
                auto stack = this->env.stack;
                if(stack.objects.empty()) {
                    Changeset passStep;
                    passStep.phaseChange = StepOrPhaseChange{true, this->env.currentPhase};
                    this->applyChangeset(passStep);
                }
                else{
                    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>,
                                 std::reference_wrapper<Ability>> top = this->env.stack.objects.back();
                    Changeset resolveSpellAbility = getBaseClass<HasEffect>(top).applyEffect(this->env);;
                    if(std::reference_wrapper<Card>* pCard = std::get_if<std::reference_wrapper<Card>>(&top))
                    {
                        bool isPermanent = false;
                        for(CardType type : ((Card&)*pCard).type){
                            if(type < PERMANENTEND && type > PERMANENTBEGIN){
                                resolveSpellAbility.moves.push_back(ObjectMovement{((Card&)*pCard).id, stack.id, this->env.battlefield.id});
                                isPermanent = true;
                            }
                        }
                        if(!isPermanent){
                            resolveSpellAbility.moves.push_back(ObjectMovement{((Card&)*pCard).id, stack.id, this->env.graveyard.id});
                        }
                    }
                    else{
                        xg::Guid id = getBaseClass<Targetable>(top).id;
                        resolveSpellAbility.remove.push_back(RemoveObject{id, stack.id});
                    }
                    applyChangeset(resolveSpellAbility);
                }
            }
            if(firstPlayerToPass == -1) {
                firstPlayerToPass = this->env.currentPlayer;
            }
            this->env.currentPlayer = nextPlayer;
        }
    }
}

void Runner::applyChangeset(Changeset& changeset) {
    // CodeReview: Apply changesets
    for(ObjectMovement& om : changeset.moves) {
        ZoneInterface& source = (ZoneInterface&)this->env.gameObjects[om.sourceZone];
        ZoneInterface& dest = (ZoneInterface&)this->env.gameObjects[om.destinationZone];
    }
    for(AddPlayerCounter& apc : changeset.playerCounters) {
        if(apc.amount < 0 && this->env.playerCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -this->env.playerCounters[apc.player][apc.counterType];
        }
        this->env.playerCounters[apc.player][apc.counterType] += apc.amount;
    }
    for(AddPermanentCounter& apc : changeset.permanentCounters) {
        if(apc.amount < 0 && this->env.permanentCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -this->env.permanentCounters[apc.player][apc.counterType];
        }
        this->env.permanentCounters[apc.player][apc.counterType] += apc.amount;
    }
    for(ObjectCreation& oc : changeset.create){
    }
    for(RemoveObject& ro : changeset.remove) {
    }
    for(LifeTotalChange& ltc : changeset.lifeTotalChanges){
        ltc.oldValue = this->env.lifeTotals[ltc.player];
        this->env.lifeTotals[ltc.player] = ltc.newValue;
    }
    for(std::reference_wrapper<EventHandler> eh : changeset.eventsToAdd){
        // CodeReview: Handle triggers/replacement effects
        this->env.triggerHandlers.push_back(eh);
    }
    for(std::reference_wrapper<EventHandler> eh : changeset.eventsToRemove){
    }
    for(std::reference_wrapper<StateQueryHandler> sqh : changeset.propertiesToAdd){
        this->env.stateQueryHandlers.push_back(sqh);
    }
    for(std::reference_wrapper<StateQueryHandler> sqh : changeset.propertiesToRemove){
    }
    for(xg::Guid& g : changeset.loseTheGame){
    }
    for(AddMana& am : changeset.addMana){
        this->env.manaPools[am.player] += am.amount;
    }
    for(RemoveMana& rm : changeset.removeMana){
        this->env.manaPools[rm.player] -= rm.amount;
    }
    if(changeset.phaseChange.changed){
        if(this->env.currentPhase == CLEANUP){
            unsigned int nextPlayer = ( this->env.turnPlayer + 1 ) % this->env.players.size();
            this->env.currentPlayer = nextPlayer;
            this->env.turnPlayer = nextPlayer;
            this->env.currentPhase = UNTAP;
        }
        else{
            this->env.currentPhase = (StepOrPhase)((int)this->env.currentPhase + 1);
        }
    }
}
