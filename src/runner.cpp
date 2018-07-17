#include "card.h"
#include "changeset.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"
#include "targeting.h"

std::variant<std::monostate, Changeset> Runner::checkStateBasedActions() {
	bool apply = false;
	Changeset stateBasedAction;
	for (std::shared_ptr<Player>& player : this->env.players) {
		// 704.5a. If a player has 0 or less life, that player loses the game.
		if (this->env.lifeTotals[player->id] <= 0) {
			apply = true;
			stateBasedAction.loseTheGame.push_back(player->id);
		}
		// 704.5c. If a player has ten or more poison counters, that player loses the game. 
		else if (this->env.playerCounters[player->id][POISONCOUNTER] >= 10) {
			apply = true;
			stateBasedAction.loseTheGame.push_back(player->id);
		}
	}

	// CodeReview: Technically milling out should happen here not in the draw function

	// 704.5d If a token is in a zone other than the battlefield, it ceases to exist
	// 704.5e. If a copy of a spell is in a zone other than the stack, it ceases to exist. If a copy of a card is in any zone other than the stack or the battlefield, it ceases to exist.
	// Ignores the stack since Tokens on the stack are assumed to be spell copies
	for (auto& pair : this->env.hands) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<Token>* token = std::get_if<std::shared_ptr<Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& pair : this->env.libraries) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<Token>* token = std::get_if<std::shared_ptr<Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& pair : this->env.graveyards) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<Token>* token = std::get_if<std::shared_ptr<Token>>(&variant)){
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& variant : this->env.battlefield->objects)
		if (std::shared_ptr<Token>* token = std::get_if<std::shared_ptr<Token>>(&variant)){
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, this->env.battlefield->id });
			apply = true;
		}
	for (auto& variant : this->env.exile->objects)
		if (std::shared_ptr<Token>* token = std::get_if<std::shared_ptr<Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, this->env.exile->id });
			apply = true;
		}

	for (auto& variant : this->env.battlefield->objects) {
		std::shared_ptr<CardToken> card = getBaseClassPtr<CardToken>(variant);
		std::shared_ptr<std::set<CardType>> types = this->env.getTypes(card);
		std::shared_ptr<std::set<CardSubType>> subtypes = this->env.getSubTypes(card);
		if(types->find(CREATURE) != types->end()) {
			int toughness = this->env.getToughness(card);
			// 704.5f. If a creature has toughness 0 or less, it's put into its owner's graveyard. Regeneration can't replace this event.
			if (toughness <= 0) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards[card->owner]->id });
				apply = true;
			}
			// 704.5g.If a creature has toughness greater than 0, and the total damage marked on it is greater than or equal to its toughness, that creature has been dealt lethal damage and is destroyed.Regeneration can replace this event.	
			else if (toughness <= this->env.damage[card->id]) {
				// CodeReview: Make a destroy change
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards[card->owner]->id });
				apply = true;
			}
			// CodeReview: Deal with deathtouch
		}
		if (types->find(PLANESWALKER) != types->end()) {
			// 704.5i. If a planeswalker has loyalty 0, it's put into its owner's graveyard.
			if (this->env.permanentCounters[card->id][LOYALTY] == 0) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards[card->owner]->id });
				apply = true;
			}
		}
		// 704.5m. If an Aura is attached to an illegal object or player, or is not attached to an object or player, that Aura is put into its owner's graveyard.
		if (subtypes->find(AURA) != subtypes->end()) {
			if (!card->targeting->validTargets(this->env.targets[card->id], this->env)) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards[card->owner]->id });
				apply = true;
			}
		}
		// CodeReview: Handle equipment being attached illegally
		// CodeReview: Handle removing things that aren't Aura's, Equipment, Fortifications that are attached or are creatures
		// CodeReview: Handle the Rasputin ability(704.5r)
		// CodeReview: Handle Sagas
	}
	// CodeReview: Handle legends rule
	// CodeReview: Handle world cards

	for (auto& pair : this->env.permanentCounters) {
		// 704.5q. If a permanent has both a +1/+1 counter and a -1/-1 counter on it, N +1/+1 and N -1/-1 counters are removed from it, where N is the smaller of the number of +1/+1 and -1/-1 counters on it.
		if (pair.second[PLUSONEPLUSONECOUNTER] > 0 && pair.second[MINUSONEMINUSONECOUNTER] > 0) {
			int amount = (int)std::min(pair.second[PLUSONEPLUSONECOUNTER], pair.second[MINUSONEMINUSONECOUNTER]);
			stateBasedAction.permanentCounters.push_back(AddPermanentCounter{ pair.first, PLUSONEPLUSONECOUNTER, -amount });
			stateBasedAction.permanentCounters.push_back(AddPermanentCounter{ pair.first, MINUSONEMINUSONECOUNTER, -amount });
			apply = true;
		}
	}

	if (apply)  return stateBasedAction;
	return {};
}

std::variant<Changeset, PassPriority> Runner::executeStep() {
	std::variant<std::monostate, Changeset> actions = this->checkStateBasedActions();
	if(Changeset* sba = std::get_if<Changeset>(&actions)) {
		return *sba;
	}

	if (!this->env.triggers.empty()) {
		// CodeReview: APNAP order and choices to be made here
		Changeset applyTriggers;
		for (QueueTrigger& trigger : this->env.triggers) {
			std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(this->env.gameObjects[trigger.player]);
			std::shared_ptr<Ability> ability = trigger.ability->clone();
			ability->id = xg::newGuid();
			ability->owner = player->id;
			// CodeReview: Do we need to set source?
			applyTriggers.create.push_back(ObjectCreation{ this->env.stack->id, trigger.ability });
			std::vector<xg::Guid> targets = player->strategy->chooseTargets(ability, *player, env);
			if (!targets.empty()) {
				applyTriggers.target.push_back(CreateTargets{ ability->id, targets });
			}
		}
		this->env.triggers.clear();
		return applyTriggers;
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
	this->env = Environment(players, libraries);
	Changeset startDraw;
	for (std::shared_ptr<Player> player : this->env.players) {
		startDraw += Changeset::drawCards(player->id, 7, this->env);
	}
	this->applyChangeset(startDraw);
	// CodeReview: Handle mulligans

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
                        for(CardType type : *this->env.getTypes(card)){
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

bool Runner::applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied) {
	for (std::shared_ptr<EventHandler> eh : this->env.replacementEffects) {
		if (applied.find(eh->id) != applied.end()) continue;
		auto result = eh->handleEvent(changeset, this->env);
		if (std::vector<Changeset>* pChangeset = std::get_if<std::vector<Changeset>>(&result)) {
			std::vector<Changeset>& changes = *pChangeset;
			applied.insert(eh->id);
			for (Changeset& change : changes) {
				if (!this->applyReplacementEffects(change, applied)) this->applyChangeset(change, false);
			}
			return true;
		}
	}
	return false;
}

void Runner::applyChangeset(Changeset& changeset, bool replacementEffects) {
#ifdef DEBUG
//	std::cout << changeset;
#endif
	// CodeReview: Reevaluate the Replacement effect system for recursion
	// CodeReview: Allow strategy to specify order to evaluate in
	// CodeReview: Replacement effect for less than 0 damage to not happen
	if (replacementEffects && this->applyReplacementEffects(changeset)) return;

	for (std::shared_ptr<EventHandler> eh : this->env.triggerHandlers) {
		auto changePrelim = eh->handleEvent(changeset, this->env);
		if (std::vector<Changeset>* pChangeset = std::get_if<std::vector<Changeset>>(&changePrelim)) {
			std::vector<Changeset>& changes = *pChangeset;
			// This should only happen with replacement effects
			if (changes.empty()) return;
			Changeset newChange;
			for (Changeset& change : changes) {
				newChange += change;
			}
			changeset = newChange;
		}
	}
    for(AddPlayerCounter& apc : changeset.playerCounters) {
        if(apc.amount < 0 && this->env.playerCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.playerCounters[apc.player][apc.counterType];
        }
        this->env.playerCounters[apc.player][apc.counterType] += apc.amount;
    }
    for(AddPermanentCounter& apc : changeset.permanentCounters) {
        if(apc.amount < 0 && this->env.permanentCounters[apc.target][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.permanentCounters[apc.target][apc.counterType];
        }
        this->env.permanentCounters[apc.target][apc.counterType] += apc.amount;
    }
    for(ObjectCreation& oc : changeset.create){
        std::shared_ptr<ZoneInterface> zone = std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects[oc.zone]);
        xg::Guid id = oc.created->id;
        zone->addObject(oc.created, id);
		if (!oc.created) {
			std::cout << "Creating a null object" << std::endl;
		}
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
	for (std::shared_ptr<EventHandler> eh : changeset.effectsToAdd) {
		this->env.replacementEffects.push_back(eh);
	}
	for (std::shared_ptr<EventHandler> eh : changeset.effectsToRemove) {
		std::vector<std::shared_ptr<EventHandler>>& list = this->env.replacementEffects;
		list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<EventHandler> e) ->
			bool { return *e == *eh; }), list.end());
	}
    for(std::shared_ptr<TriggerHandler> eh : changeset.triggersToAdd){
        this->env.triggerHandlers.push_back(eh);
    }
    for(std::shared_ptr<TriggerHandler> eh : changeset.triggersToRemove){
        std::vector<std::shared_ptr<TriggerHandler>>& list = this->env.triggerHandlers;
        list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<TriggerHandler> e) ->
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
	for (QueueTrigger& qt : changeset.trigger) {
		this->env.triggers.push_back(qt);
	}
    for(ObjectMovement& om : changeset.moves) {
        ZoneInterface& source = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.sourceZone));
        ZoneInterface& dest = *std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects.at(om.destinationZone));

        std::shared_ptr<Targetable> object = source.removeObject(om.object);
        om.newObject = dest.addObject(object, om.newObject);
		this->env.gameObjects.erase(om.object);
		if (!object) {
			std::cout << "Got a null move" << std::endl;
		}
		this->env.gameObjects[om.newObject] = object;
    }
	// CodeReview: Handle losing the game
	for (xg::Guid& ltg : changeset.loseTheGame) {
		unsigned int index = 0;
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
 : libraries(libraries), players(players), env(players, libraries)
{
    if(players.size() != libraries.size()) {
#ifdef DEBUG
        std::cerr << "Not equal players and libraries" << std::endl;
#endif
        throw "Not equal players and libraries";
    }
}
