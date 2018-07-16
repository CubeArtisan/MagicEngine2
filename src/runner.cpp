#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"

std::variant<Changeset, PassPriority> Runner::executeStep()
{
    // CodeReview: Check state based actions
    Player& active = (Player&)this->env.players[this->env.currentPlayer];
    GameAction action = active.strategy->chooseGameAction(active, env);

    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        Changeset castSpell;
        Targetable hand = this->env.hands[active.id];
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand.id, this->env.stack.id});
		std::shared_ptr<Card> spell = std::dynamic_pointer_cast<Card>(this->env.gameObjects[pCastSpell->spell]);
        // CodeReview: Assign targets
        castSpell += pCastSpell->cost.payCost(active, env, spell);
        for(std::shared_ptr<Cost> c : pCastSpell->additionalCosts) {
            castSpell += c->payCost(active, env, spell);
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
        std::shared_ptr<ActivatedAbility> result = pActivateAnAbility->ability;
        result->source = pActivateAnAbility->source;
        result->id = xg::newGuid();
        activateAbility.create.push_back(ObjectCreation{this->env.stack.id, result});
        // CodeReview: Assign targets
        activateAbility += pActivateAnAbility->cost.payCost(active, env, result->source);
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
	int count = 1;
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
                    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>,
                                 std::shared_ptr<Ability>> top = this->env.stack.objects.back();
                    Changeset resolveSpellAbility = getBaseClassPtr<HasEffect>(top)->applyEffect(this->env);;
                    if(std::shared_ptr<Card>* pCard = std::get_if<std::shared_ptr<Card>>(&top))
                    {
						std::shared_ptr<Card> card = *pCard;
                        bool isPermanent = false;
                        for(CardType type : card->baseTypes){
                            if(type < PERMANENTEND && type > PERMANENTBEGIN){
                                resolveSpellAbility.moves.push_back(ObjectMovement{card->id, stack.id, this->env.battlefield.id});
                                isPermanent = true;
                            }
                        }
                        if(!isPermanent){
                            resolveSpellAbility.moves.push_back(ObjectMovement{card->id, stack.id, this->env.graveyard.id});
                        }
                    }
                    else{
                        xg::Guid id = getBaseClassPtr<Targetable>(top)->id;
                        resolveSpellAbility.remove.push_back(RemoveObject{id, stack.id});
                    }
                    applyChangeset(resolveSpellAbility);
                }
            }
            else if(firstPlayerToPass == -1) {
                firstPlayerToPass = this->env.currentPlayer;
            }
			this->env.currentPlayer = nextPlayer;
        }
		count++;
		if (count > 100) break;
    }
}

void Runner::applyChangeset(Changeset& changeset) {
#ifdef DEBUG
    std::cout << changeset;
#endif
    // CodeReview: Use the event system
    for(AddPlayerCounter& apc : changeset.playerCounters) {
        if(apc.amount < 0 && this->env.playerCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.playerCounters[apc.player][apc.counterType];
        }
        this->env.playerCounters[apc.player][apc.counterType] += apc.amount;
    }
    for(AddPermanentCounter& apc : changeset.permanentCounters) {
        if(apc.amount < 0 && this->env.permanentCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.permanentCounters[apc.player][apc.counterType];
        }
        this->env.permanentCounters[apc.player][apc.counterType] += apc.amount;
    }
    for(ObjectCreation& oc : changeset.create){
        std::shared_ptr<ZoneInterface> zone = std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects[oc.zone]);
        xg::Guid id = oc.created->id;
        zone->addObject(oc.created, id);
        this->env.gameObjects[id] = oc.created;
    }
    for(RemoveObject& ro : changeset.remove) {
		std::shared_ptr<ZoneInterface> zone = std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects[ro.zone]);
		zone->removeObject(ro.object);
		this->env.gameObjects.erase(ro.object);
    }
    for(LifeTotalChange& ltc : changeset.lifeTotalChanges){
        ltc.oldValue = this->env.lifeTotals[ltc.player];
        this->env.lifeTotals[ltc.player] = ltc.newValue;
    }
    for(std::shared_ptr<EventHandler> eh : changeset.eventsToAdd){
        // CodeReview: Handle triggers/replacement effects
        this->env.triggerHandlers.push_back(eh);
    }
    for(std::shared_ptr<EventHandler> eh : changeset.eventsToRemove){
        // CodeReview: Handle triggers/replacement effects
        std::vector<std::shared_ptr<EventHandler>>& list = this->env.triggerHandlers;
        list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<EventHandler> e) ->
                                                            bool { return *e == *eh; }), list.end());
    }
    for(std::shared_ptr<StateQueryHandler> sqh : changeset.propertiesToAdd){
        this->env.stateQueryHandlers.push_back(sqh);
    }
    for(std::shared_ptr<StateQueryHandler> sqh : changeset.propertiesToRemove){
        std::vector<std::shared_ptr<StateQueryHandler>>& list = this->env.stateQueryHandlers;
        list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<StateQueryHandler> e) ->
                                                            bool { return *e == *sqh; }), list.end());
    }
    for(AddMana& am : changeset.addMana){
        this->env.manaPools[am.player] += am.amount;
    }
    for(RemoveMana& rm : changeset.removeMana){
        this->env.manaPools[rm.player] -= rm.amount;
    }
    for(DamageToTarget& dtt : changeset.damage){
        std::shared_ptr<Targetable> pObject = this->env.gameObjects[dtt.target];
        Targetable& object = *pObject;
        if(std::type_index(typeid(Player)) == std::type_index(typeid(object)))
        {
            Player player = (Player&)object;
            int lifeTotal = this->env.lifeTotals[object.id];
            Changeset lifeLoss;
            lifeLoss.lifeTotalChanges.push_back(LifeTotalChange{object.id, lifeTotal - (int)dtt.amount, lifeTotal});
        }
        // CodeReview: Handle other cases
    }
	for (TapTarget& tt : changeset.tap) {
		std::shared_ptr<Targetable> object = this->env.gameObjects[tt.target];
		std::shared_ptr<CardToken> pObject = std::dynamic_pointer_cast<CardToken>(object);
		pObject->is_tapped = tt.tap;
	}
    if(changeset.phaseChange.changed){
		for (auto& manaPool : this->env.manaPools) {
			manaPool.second.clear();
		}
		if (this->env.currentPhase == END) {
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer].id;
			if (this->env.hands[turnPlayerId].objects.size() > 7) {
				// CodeReview: Implement discarding
			}
			this->env.currentPhase = CLEANUP;
			// CodeReview: Remove marked damage
			Changeset cleanup;
			// CodeReview: Only do if nothing is on the stack
			cleanup.phaseChange = StepOrPhaseChange{ true, CLEANUP };
			this->applyChangeset(cleanup);
		}
		else if(this->env.currentPhase == CLEANUP) {
            unsigned int nextPlayer = ( this->env.turnPlayer + 1 ) % this->env.players.size();
            this->env.currentPlayer = nextPlayer;
            this->env.turnPlayer = nextPlayer;
            this->env.currentPhase = UNTAP;
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer].id;
			
			Changeset untap;
			for (auto& object : this->env.battlefield.objects) {
				std::shared_ptr<CardToken> card = getBaseClassPtr<CardToken>(object);
				if (card->owner == turnPlayerId && card->is_tapped) {
					untap.tap.push_back(TapTarget{ card->id, false });
				}
			}
			untap.phaseChange = StepOrPhaseChange{ true, UNTAP };
			this->applyChangeset(untap);
        }
        else{
            this->env.currentPhase = (StepOrPhase)((int)this->env.currentPhase + 1);
        }

		if (this->env.currentPhase == DRAW) {
			Changeset drawCard = Changeset::drawCards(this->env.players[this->env.turnPlayer].id, 1, env);
			this->applyChangeset(drawCard);
		}
    }
    for(ObjectMovement& om : changeset.moves) {
        ZoneInterface& source = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.sourceZone));
        ZoneInterface& dest = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.destinationZone));

        std::shared_ptr<Targetable> object = source.removeObject(om.object);
        om.newObject = dest.addObject(object, om.newObject);
		this->env.gameObjects[om.newObject] = object;
		// CodeReview: Figure out why this line corrupts the shared_ptr
		// this->env.gameObjects.erase(om.object);
    }
	// CodeReview: Handle losing the game
    // for(xg::Guid& g : changeset.loseTheGame){
    // }

    this->env.changes.push_back(changeset);
}

Runner::Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players)
 : env(players, libraries)
{
    if(players.size() != libraries.size()) {
#ifdef DEBUG
        std::cerr << "Not equal players and libraries" << std::endl;
#endif
        throw "Not equal players and libraries";
    }

    Changeset startDraw;
    for(Player& player : this->env.players) {
        startDraw += Changeset::drawCards(player.id, 7, this->env);
    }
    this->applyChangeset(startDraw);

    this->runGame();
}
