#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"

std::variant<std::monostate, Changeset> Runner::checkStateBasedActions() {
	bool lost = false;
	Changeset loseTheGame;
	for (std::shared_ptr<Player>& player : this->env.players) {
		if (this->env.lifeTotals[player->id] <= 0) {
			lost = true;
			loseTheGame.loseTheGame.push_back(player->id);
		}
	}
	if (lost) return loseTheGame;

	bool apply = false;
	Changeset stateBasedAction;
	for (auto& variant : this->env.battlefield->objects) {
		std::shared_ptr<CardToken> card = getBaseClassPtr<CardToken>(variant);
		std::set<CardType> types = this->env.getTypes(card);
		if(types.find(CREATURE) != types.end()){
			int toughness = this->env.getToughness(card->id);
			int damage = this->env.damage[card->id];
			if (this->env.getToughness(card->id) <= this->env.damage[card->id]) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards[card->owner]->id });
				apply = true;
			}
		}
	}

	if (apply)  return stateBasedAction;
	return {};
}

std::variant<Changeset, PassPriority> Runner::executeStep() {
    // CodeReview: Check state based actions
	std::variant<std::monostate, Changeset> actions = this->checkStateBasedActions();
	if(Changeset* sba = std::get_if<Changeset>(&actions)) {
		return *sba;
	}

	Player& active = *this->env.players[this->env.currentPlayer];
	GameAction action = active.strategy->chooseGameAction(active, env);
    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        Changeset castSpell;
        std::shared_ptr<Zone<Card, Token>> hand = this->env.hands.at(active.id);
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand->id, this->env.stack->id});
		std::shared_ptr<Card> spell = std::dynamic_pointer_cast<Card>(this->env.gameObjects[pCastSpell->spell]);
#ifdef DEBUG
		std::cout << "Casting " << spell->name << std::endl;
#endif
		if (pCastSpell->targets.size() > 0) {
			castSpell.target.push_back(CreateTargets{ castSpell.moves[0].newObject, pCastSpell->targets });
		}
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
        std::shared_ptr<Zone<Card, Token>> hand = this->env.hands.at(active.id);
        playLand.moves.push_back(ObjectMovement{pPlayLand->land, hand->id, this->env.battlefield->id});
        // CodeReview: Use land play for the turn
        return playLand;
    }

    if(ActivateAnAbility* pActivateAnAbility = std::get_if<ActivateAnAbility>(&action)){
        Changeset activateAbility;
        std::shared_ptr<ActivatedAbility> result =
			std::dynamic_pointer_cast<ActivatedAbility>(pActivateAnAbility->ability->clone());
        result->source = pActivateAnAbility->source;
		result->owner = active.id;
        result->id = xg::newGuid();
        activateAbility.create.push_back(ObjectCreation{this->env.stack->id, result});
		if (pActivateAnAbility->targets.size() > 0) {
			activateAbility.target.push_back(CreateTargets{ result->id, pActivateAnAbility->targets });
		}
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
    while(this->env.players.size() > 1) {
        std::variant<Changeset, PassPriority> step = this->executeStep();

        if(Changeset* pChangeset = std::get_if<Changeset>(&step)){
            this->applyChangeset(*pChangeset);
            firstPlayerToPass = -1;
        }
        else {
            int nextPlayer = (this->env.currentPlayer + 1) % this->env.players.size();
            if(firstPlayerToPass == nextPlayer){
                auto stack = this->env.stack;
                if(stack->objects.empty()) {
                    Changeset passStep;
                    passStep.phaseChange = StepOrPhaseChange{true, this->env.currentPhase};
                    this->applyChangeset(passStep);
                }
                else{
                    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>,
                                 std::shared_ptr<Ability>> top = this->env.stack->objects.back();
                    Changeset resolveSpellAbility = getBaseClassPtr<HasEffect>(top)->applyEffect(this->env);
                    if(std::shared_ptr<Card>* pCard = std::get_if<std::shared_ptr<Card>>(&top)) {
						std::shared_ptr<Card> card = *pCard;
                        bool isPermanent = false;
                        for(CardType type : this->env.getTypes(card)){
                            if(type < PERMANENTEND && type > PERMANENTBEGIN){
                                resolveSpellAbility.moves.push_back(ObjectMovement{card->id, stack->id, this->env.battlefield->id});
                                isPermanent = true;
								break;
                            }
                        }
                        if(!isPermanent){
                            resolveSpellAbility.moves.push_back(ObjectMovement{card->id, stack->id, this->env.graveyards.at(card->owner)->id});
                        }
                    }
                    else {
                        xg::Guid id = getBaseClassPtr<Targetable>(top)->id;
                        resolveSpellAbility.remove.push_back(RemoveObject{id, stack->id});
                    }
                    applyChangeset(resolveSpellAbility);
                }
            }
            else if(firstPlayerToPass == -1) {
                firstPlayerToPass = this->env.currentPlayer;
            }
			this->env.currentPlayer = nextPlayer;
        }
    }
}

void Runner::applyChangeset(Changeset& changeset) {
#ifdef DEBUG
    std::cout << changeset;
#endif
    // CodeReview: Reevaluate the Replacement effect system for recursion
	// CodeReview: Allow strategy to specify order to evaluate in
	// CodeReview: Replacement effect for less than 0 damage to not happen
	for (std::shared_ptr<EventHandler> eh : this->env.replacementEffects) {
		std::vector<Changeset> changes = eh->handleEvent(changeset, this->env);
		if (changes.empty()) return;
		Changeset newChange;
		for (Changeset& change : changes) {
			newChange += change;
		}
		changeset = newChange;
	}
	for (std::shared_ptr<EventHandler> eh : this->env.triggerHandlers) {
		std::vector<Changeset> changes = eh->handleEvent(changeset, this->env);
		// This should only happen with replacement effects
		if (changes.empty()) return;
		Changeset newChange;
		for (Changeset& change : changes) {
			newChange += change;
		}
		changeset = newChange;
	}
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
        // CodeReview: Handle triggers/replacement effects differences
        this->env.triggerHandlers.push_back(eh);
    }
    for(std::shared_ptr<EventHandler> eh : changeset.eventsToRemove){
        // CodeReview: Handle triggers/replacement effects differences
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
    for(AddMana& am : changeset.addMana) {
        this->env.manaPools.at(am.player) += am.amount;
    }
    for(RemoveMana& rm : changeset.removeMana) {
        this->env.manaPools.at(rm.player) -= rm.amount;
    }
    for(DamageToTarget& dtt : changeset.damage) {
        std::shared_ptr<Targetable> pObject = this->env.gameObjects[dtt.target];
        if(std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(pObject)) {
            int lifeTotal = this->env.lifeTotals[player->id];
            Changeset lifeLoss;
            lifeLoss.lifeTotalChanges.push_back(LifeTotalChange{player->id, lifeTotal, lifeTotal - (int)dtt.amount});
			applyChangeset(lifeLoss);
		}
		else if (std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(pObject)) {
#ifdef DEBUG
			std::cout << dtt.amount << " dealt to " << card->name << " " << card->id << std::endl;
#endif
			this->env.damage[card->id] += dtt.amount;
		}
    }
	for (TapTarget& tt : changeset.tap) {
		std::shared_ptr<Targetable> object = this->env.gameObjects[tt.target];
		std::shared_ptr<CardToken> pObject = std::dynamic_pointer_cast<CardToken>(object);
		pObject->is_tapped = tt.tap;
	}
	for (CreateTargets& ct : changeset.target) {
		this->env.targets[ct.object] = ct.targets;
	}
    if(changeset.phaseChange.changed){
		changeset.phaseChange.starting = this->env.currentPhase;
		// CodeReview: How to handle extra steps?
		// CodeReview: Handle mana that doesn't empty
		for (auto& manaPool : this->env.manaPools) {
			manaPool.second.clear();
		}
		if (this->env.currentPhase == END) {
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			this->env.currentPhase = CLEANUP;
			// CodeReview: Do this with Hand Size from StateQuery
			if (this->env.hands.at(turnPlayerId)->objects.size() > 7) {
				// CodeReview: Implement discarding
			}
			this->env.damage.clear();
			Changeset cleanup;
			// CodeReview: Only do if nothing is on the stack
			cleanup.phaseChange = StepOrPhaseChange{ true, CLEANUP };
			this->applyChangeset(cleanup);
		}
		else if(this->env.currentPhase == CLEANUP) {
			// CodeReview: Get next player from Environment
            unsigned int nextPlayer = ( this->env.turnPlayer + 1 ) % this->env.players.size();
			this->env.currentPlayer = nextPlayer;
            this->env.turnPlayer = nextPlayer;
            this->env.currentPhase = UNTAP;
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			
			Changeset untap;
			untap.tap.reserve(this->env.battlefield->objects.size());
			for (auto& object : this->env.battlefield->objects) {
				std::shared_ptr<CardToken> card = getBaseClassPtr<CardToken>(object);
				if (this->env.getController(card->id) == turnPlayerId && card->is_tapped) {
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
			Changeset drawCard = Changeset::drawCards(this->env.players[this->env.turnPlayer]->id, 1, env);
			this->applyChangeset(drawCard);
		}
    }
    for(ObjectMovement& om : changeset.moves) {
        ZoneInterface& source = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.sourceZone));
        ZoneInterface& dest = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.destinationZone));

        std::shared_ptr<Targetable> object = source.removeObject(om.object);
        om.newObject = dest.addObject(object, om.newObject);
		this->env.gameObjects.erase(om.object);
		this->env.gameObjects[om.newObject] = object;
    }
	// CodeReview: Handle losing the game
	for (xg::Guid& ltg : changeset.loseTheGame) {
		int index = 0;
		for (auto iter = this->env.players.begin(); iter != this->env.players.end(); iter++) {
			if ((*iter)->id == ltg) {
				if (env.turnPlayer == index)
				{
					// CodeReview: Exile everything on the stack
					env.currentPhase = END;
					Changeset endTurn;
					endTurn.phaseChange = StepOrPhaseChange{ true, END };
					applyChangeset(endTurn);
				}
				if (env.currentPlayer == index) env.currentPlayer = (env.currentPlayer + 1) % env.players.size();
				// CodeReview remove unneeded data structures
				// CodeReview remove eventhandlers registered to that player
				// Need a way to find what zone an object is in to do this for all objects
				// This isn't working currently for an unknown reason
				Changeset removeCards;
				for (auto& card : this->env.battlefield->objects) {
					if (getBaseClassPtr<Targetable>(card)->owner == ltg) {
						removeCards.remove.push_back(RemoveObject{ getBaseClassPtr<Targetable>(card)->id, this->env.battlefield->id });
					}
				}
				applyChangeset(removeCards);
				env.players.erase(iter);
				break;
			}
			index++;
		}
	}

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
    for(std::shared_ptr<Player> player : this->env.players) {
        startDraw += Changeset::drawCards(player->id, 7, this->env);
    }
    this->applyChangeset(startDraw);
	// CodeReview: Handle mulligans
}
