#include "card.h"
#include "changeset.h"
#include "combat.h"
#include "player.h"
#include "runner.h"
#include "gameAction.h"
#include "targeting.h"
#include "util.h"

std::variant<std::monostate, Changeset> Runner::checkStateBasedActions() const {
	bool apply = false;
	Changeset stateBasedAction;
	for (const std::shared_ptr<Player>& player : this->env.players) {
		// 704.5a. If a player has 0 or less life, that player loses the game.
		if (this->env.lifeTotals.at(player->id) <= 0) {
			apply = true;
			stateBasedAction.loseTheGame.push_back(player->id);
		}
		// 704.5c. If a player has ten or more poison counters, that player loses the game. 
		else if (this->env.playerCounters.at(player->id).at(POISONCOUNTER) >= 10) {
			apply = true;
			stateBasedAction.loseTheGame.push_back(player->id);
		}
	}

	// CodeReview: Technically milling out should happen here not in the draw function

	// 704.5d If a token is in a zone other than the battlefield, it ceases to exist
	// 704.5e. If a copy of a spell is in a zone other than the stack, it ceases to exist. If a copy of a card is in any zone other than the stack or the battlefield, it ceases to exist.
	// Ignores the stack since Tokens on the stack are assumed to be spell copies
	for (auto& pair : this->env.hands) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& pair : this->env.libraries) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& pair : this->env.graveyards) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)){
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& variant : this->env.exile->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.remove.push_back(RemoveObject{ (*token)->id, this->env.exile->id });
			apply = true;
		}

	for (auto& variant : this->env.battlefield->objects) {
		std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(variant);
		std::shared_ptr<const std::set<CardType>> types = this->env.getTypes(card);
		std::shared_ptr<const std::set<CardSubType>> subtypes = this->env.getSubTypes(card);
		if(types->find(CREATURE) != types->end()) {
			int toughness = this->env.getToughness(card);
			// 704.5f. If a creature has toughness 0 or less, it's put into its owner's graveyard. Regeneration can't replace this event.
			if (toughness <= 0) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id });
				apply = true;
			}
			// 704.5g.If a creature has toughness greater than 0, and the total damage marked on it is greater than or equal to its toughness, that creature has been dealt lethal damage and is destroyed.Regeneration can replace this event.	
			else if (toughness <= tryAtMap(this->env.damage, card->id, 0)) {
				// CodeReview: Make a destroy change
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id });
				apply = true;
			}
			// CodeReview: Deal with deathtouch
		}
		if (types->find(PLANESWALKER) != types->end()) {
			// 704.5i. If a planeswalker has loyalty 0, it's put into its owner's graveyard.
			// CodeReview: Assumes all planeswalkers will have an entry for loyalty counters in the map
			if (this->env.permanentCounters.at(card->id).at(LOYALTY) == 0) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id });
				apply = true;
			}
		}
		// 704.5m. If an Aura is attached to an illegal object or player, or is not attached to an object or player, that Aura is put into its owner's graveyard.
		if (subtypes->find(AURA) != subtypes->end()) {
			if (!card->targeting->validTargets(this->env.targets.at(card->id), this->env)) {
				stateBasedAction.moves.push_back(ObjectMovement{ card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id });
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
		// CodeReview: Assumes every permanent with any counters will have entries for both in the map
		if (tryAtMap(pair.second, PLUSONEPLUSONECOUNTER, 0u) > 0 && tryAtMap(pair.second, MINUSONEMINUSONECOUNTER, 0u) > 0) {
			int amount = (int)std::min(pair.second.at(PLUSONEPLUSONECOUNTER), pair.second.at(MINUSONEMINUSONECOUNTER));
			stateBasedAction.permanentCounters.push_back(AddPermanentCounter{ pair.first, PLUSONEPLUSONECOUNTER, -amount });
			stateBasedAction.permanentCounters.push_back(AddPermanentCounter{ pair.first, MINUSONEMINUSONECOUNTER, -amount });
			apply = true;
		}
	}

	if (apply)  return stateBasedAction;
	return {};
}

std::variant<Changeset, PassPriority> Runner::executeStep() const {
	// 116.5. Each time a player would get priority, the game first performs all applicable state-based actions as a
	// single event (see rule 704, "State-Based Actions"), then repeats this process until no state-based actions are
	// performed. Then triggered abilities are put on the stack (see rule 603, "Handling Triggered Abilities"). These
	// steps repeat in order until no further state-based actions are performed and no abilities trigger. Then the
	// player who would have received priority does so.
	std::variant<std::monostate, Changeset> actions = this->checkStateBasedActions();
	if(Changeset* sba = std::get_if<Changeset>(&actions)) {
		return *sba;
	}

	if (!this->env.triggers.empty()) {
		// CodeReview: APNAP order and choices to be made here
		Changeset applyTriggers;
		for (const QueueTrigger& trigger : this->env.triggers) {
			std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(this->env.gameObjects.at(trigger.player));
			std::shared_ptr<Ability> ability = trigger.ability->clone();
			ability->id = xg::newGuid();
			ability->owner = player->id;
			// CodeReview: Do we need to set source?
			applyTriggers.create.push_back(ObjectCreation{ this->env.stack->id, ability });
			std::vector<xg::Guid> targets = player->strategy->chooseTargets(ability, *player, env);
			if (!targets.empty()) {
				applyTriggers.target.push_back(CreateTargets{ ability->id, targets });
			}
		}
		applyTriggers.clearTriggers = true;
		return applyTriggers;
	}

	Player& active = *this->env.players[this->env.currentPlayer];
	GameAction action = active.strategy->chooseGameAction(active, env);
    if(CastSpell* pCastSpell = std::get_if<CastSpell>(&action)){
        Changeset castSpell;
        std::shared_ptr<Zone<Card, Token>> hand = this->env.hands.at(active.id);
        castSpell.moves.push_back(ObjectMovement{pCastSpell->spell, hand->id, this->env.stack->id});
		std::shared_ptr<Card> spell = std::dynamic_pointer_cast<Card>(this->env.gameObjects.at(pCastSpell->spell));
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
		playLand.land.push_back(LandPlay{ pPlayLand->land, active.id, hand->id });
        // CodeReview: Use land play for the turn
        return playLand;
    }

    if(ActivateAnAbility* pActivateAnAbility = std::get_if<ActivateAnAbility>(&action)){
        Changeset activateAbility;
		std::shared_ptr<ActivatedAbility> result(pActivateAnAbility->ability);
		result->source = pActivateAnAbility->source;
		result->owner = active.id;
        result->id = xg::newGuid();
		if (std::shared_ptr<ManaAbility> manaAbility = std::dynamic_pointer_cast<ManaAbility>(result)) {
			activateAbility.manaAbility.push_back(manaAbility);
		}
		else {
			activateAbility.create.push_back(ObjectCreation{ this->env.stack->id, result });
		}
		if (pActivateAnAbility->targets.size() > 0) {
			activateAbility.target.push_back(CreateTargets{ result->id, pActivateAnAbility->targets });
		}
        activateAbility += pActivateAnAbility->cost.payCost(active, env, result->source);
#ifdef DEBUG
		std::cout << active.id << " is activating an ability of " << getBaseClassPtr<const CardToken>(pActivateAnAbility->source)->name << std::endl;
#endif
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
				// 500.2. A phase or step in which players receive priority ends when the stack is empty and all players
				// pass in succession.Simply having the stack become empty doesn't cause such a phase or step to end;
				// all players have to pass in succession with the stack empty. Because of this, each player gets a chance
				// to add new things to the stack before that phase or step ends.
                if(stack->objects.empty()) {
                    Changeset passStep;
                    passStep.phaseChange = StepOrPhaseChange{true, this->env.currentPhase};
                    this->applyChangeset(passStep);
                }
                else{
                    std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>,
                                 std::shared_ptr<const Ability>> top = this->env.stack->objects.back();

					// CodeReview: Call getNextApplyEffect if not nullopt applyChangeset
					// Then repeat till nullopt
					// Then move ability/card to correct zone
                    Changeset resolveSpellAbility = getBaseClassPtr<const HasEffect>(top)->applyEffect(this->env);
                    if(const std::shared_ptr<const Card>* pCard = std::get_if<std::shared_ptr<const Card>>(&top)) {
						std::shared_ptr<const Card> card = *pCard;
                        bool isPermanent = false;
                        for(CardType type : *this->env.getTypes(card)){
                            if(PERMANENTBEGIN < type && type < PERMANENTEND) {
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
                        xg::Guid id = getBaseClassPtr<const Targetable>(top)->id;
                        resolveSpellAbility.remove.push_back(RemoveObject{id, stack->id});
                    }
                    applyChangeset(resolveSpellAbility);
                }
				this->env.currentPlayer = this->env.turnPlayer;
            }
            else {
				if (firstPlayerToPass == -1) {
					firstPlayerToPass = this->env.currentPlayer;
				}
				this->env.currentPlayer = nextPlayer;
			}
        }
    }
}

void Runner::applyMoveRules(Changeset& changeset) {
	std::vector<std::tuple<std::shared_ptr<HasAbilities>, ZoneType, std::optional<ZoneType>>> objects;
	for (auto& move : changeset.moves) {
		if (std::shared_ptr<HasAbilities> object = std::dynamic_pointer_cast<HasAbilities>(env.gameObjects.at(move.object))) {
			std::shared_ptr<ZoneInterface> source = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects.at(move.sourceZone));
			std::shared_ptr<ZoneInterface> destination = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects.at(move.destinationZone));
			objects.push_back(std::make_tuple(object, destination->type, source->type));
		}
	}
	for (auto& create : changeset.create) {
		if (std::shared_ptr<HasAbilities> object = std::dynamic_pointer_cast<HasAbilities>(create.created)) {
			std::shared_ptr<ZoneInterface> destination = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects.at(create.zone));
			objects.push_back(std::make_tuple(object, destination->type, std::nullopt));
		}
	}
	std::vector<Changeset> result;
	bool apply = false;
	Changeset addStatic;
	for (auto& object : objects) {
		std::vector<std::shared_ptr<StaticEffectHandler>> handlers = env.getStaticEffects(std::get<0>(object), std::get<1>(object), std::get<2>(object));
		if (!handlers.empty()) {
			for (const auto& h : handlers) addStatic.propertiesToAdd.push_back(h);
			apply = true;
		}
	}
	if (apply) this->applyChangeset(addStatic);
	apply = false;
	// 614.12. Some replacement effects modify how a permanent enters the battlefield. (See rules 614.1c-d.) Such effects may come from the permanent itself if they
	// affect only that permanent (as opposed to a general subset of permanents that includes it). They may also come from other sources. To determine which replacement
	// effects apply and how they apply, check the characteristics of the permanent as it would exist on the battlefield, taking into account replacement effects that
	// have already modified how it enters the battlefield (see rule 616.1), continuous effects from the permanent's own static abilities that would apply to it once it's
	// on the battlefield, and continuous effects that already exist and would apply to the permanent.
	Changeset addReplacement;
	for (auto& object : objects) {
		if (std::get<1>(object) != BATTLEFIELD) continue;
		std::vector<std::shared_ptr<EventHandler>> handlers = env.getSelfReplacementEffects(std::get<0>(object), std::get<1>(object), std::get<2>(object));
		if (!handlers.empty()) {
			for (auto& h : handlers) addReplacement.effectsToAdd.push_back(h);
			apply = true;
		}
	}
	if (apply) this->applyChangeset(addReplacement);
}

bool Runner::applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied) {
	// CodeReview: Allow strategy to specify order to evaluate in
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
	this->applyMoveRules(changeset);
	if (replacementEffects && this->applyReplacementEffects(changeset)) return;
#ifdef DEBUG
	// std::cout << changeset;
#endif

    for(AddPlayerCounter& apc : changeset.playerCounters) {
        if(apc.amount < 0 && this->env.playerCounters[apc.player][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.playerCounters[apc.player][apc.counterType];
        }
        this->env.playerCounters[apc.player][apc.counterType] += apc.amount;
#ifdef DEBUG
		std::cout << "Changing " << apc.counterType << " counters on player " << apc.player
			      << " by " << apc.amount << std::endl;
#endif
    }
    for(AddPermanentCounter& apc : changeset.permanentCounters) {
        if(apc.amount < 0 && this->env.permanentCounters[apc.target][apc.counterType] < (unsigned int)-apc.amount){
            apc.amount = -(int)this->env.permanentCounters[apc.target][apc.counterType];
        }
#ifdef DEBUG
		std::cout << "Changing " << apc.counterType << " counters on " << std::dynamic_pointer_cast<CardToken>(this->env.gameObjects.at(apc.target))
				  << " by " << apc.amount << std::endl;
#endif
        this->env.permanentCounters[apc.target][apc.counterType] += apc.amount;
    }
    for(ObjectCreation& oc : changeset.create){
		if (!oc.created) {
			std::cout << "Creating a null object" << std::endl;
		}
        std::shared_ptr<ZoneInterface> zone = std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects[oc.zone]);
		xg::Guid id = oc.created->id;
#ifdef DEBUG
		// CodeReview: If is CardToken show name
		std::cout << "Creating " << id << " in " << zone->type << std::endl;
#endif
        zone->addObject(oc.created);
        this->env.gameObjects[id] = oc.created;
		if (std::shared_ptr<HasAbilities> abilities = std::dynamic_pointer_cast<HasAbilities>(oc.created)) {
			std::vector<std::shared_ptr<EventHandler>> replacement = this->env.getReplacementEffects(abilities, zone->type);
			for (auto& r : replacement) r->owner = oc.created->id;
			this->env.replacementEffects.insert(this->env.replacementEffects.end(), replacement.begin(), replacement.end());
			std::vector<std::shared_ptr<TriggerHandler>> trigger = this->env.getTriggerEffects(abilities, zone->type);
			for (auto& t : trigger) t->owner = oc.created->id;
			this->env.triggerHandlers.insert(this->env.triggerHandlers.end(), trigger.begin(), trigger.end());
		}
    }
    for(RemoveObject& ro : changeset.remove) {
		std::shared_ptr<ZoneInterface> zone = std::dynamic_pointer_cast<ZoneInterface>(this->env.gameObjects[ro.zone]);
#ifdef DEBUG
		// CodeReview: If is CardToken show name
		std::cout << "Removing " << ro.object << " from " << zone->type << std::endl;
#endif
		zone->removeObject(ro.object);
		this->env.gameObjects.erase(ro.object);
		this->env.triggerHandlers.erase(std::remove_if(this->env.triggerHandlers.begin(), this->env.triggerHandlers.end(), [&](std::shared_ptr<TriggerHandler>& a) -> bool { return a->owner == ro.object; }), this->env.triggerHandlers.end());
		this->env.replacementEffects.erase(std::remove_if(this->env.replacementEffects.begin(), this->env.replacementEffects.end(), [&](std::shared_ptr<EventHandler>& a) -> bool { return a->owner == ro.object; }), this->env.replacementEffects.end());
		this->env.stateQueryHandlers.erase(std::remove_if(this->env.stateQueryHandlers.begin(), this->env.stateQueryHandlers.end(), [&](std::shared_ptr<StaticEffectHandler>& a) -> bool { return a->owner == ro.object; }), this->env.stateQueryHandlers.end());
    }
    for(LifeTotalChange& ltc : changeset.lifeTotalChanges) {
#ifdef DEBUG
		std::cout << ltc.player << " has life total set to " << ltc.newValue << " from " << ltc.oldValue << std::endl;
#endif
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
    for(std::shared_ptr<TriggerHandler> th : changeset.triggersToAdd){
        this->env.triggerHandlers.push_back(th);
    }
    for(std::shared_ptr<TriggerHandler> th : changeset.triggersToRemove){
        std::vector<std::shared_ptr<TriggerHandler>>& list = this->env.triggerHandlers;
        list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<TriggerHandler> e) ->
                                                            bool { return *e == *th; }), list.end());
    }
    for(std::shared_ptr<StaticEffectHandler> sqh : changeset.propertiesToAdd){
		// CodeReview: if sqh is a ControlChangeHandler check if the target is not under the new controller's
		// control if so fire control change event for the target
        this->env.stateQueryHandlers.push_back(sqh);
    }
    for(std::shared_ptr<StaticEffectHandler> sqh : changeset.propertiesToRemove){
		// CodeReview: if sqh is a ControlChangeHandler get the current controller with that handler
		// Then after removing the handler check again if the controllers are different fire a control change event
        std::vector<std::shared_ptr<StaticEffectHandler>>& list = this->env.stateQueryHandlers;
        list.erase(std::remove_if(list.begin(), list.end(), [&](std::shared_ptr<StaticEffectHandler> e) ->
                                                            bool { return *e == *sqh; }), list.end());
    }
    for(AddMana& am : changeset.addMana) {
#ifdef DEBUG
		std::cout << "Adding " << am.amount << " to " << am.player << "'s mana pool" << std::endl;
#endif
        this->env.manaPools.at(am.player) += am.amount;
    }
    for(RemoveMana& rm : changeset.removeMana) {
#ifdef DEBUG
		std::cout << "Removing " << rm.amount << " from " << rm.player << "'s mana pool" << std::endl;
#endif
        this->env.manaPools.at(rm.player) -= rm.amount;
    }
    for(DamageToTarget& dtt : changeset.damage) {
        std::shared_ptr<Targetable> pObject = this->env.gameObjects[dtt.target];
        if(std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(pObject)) {
            int lifeTotal = this->env.lifeTotals[player->id];
            Changeset lifeLoss;
            lifeLoss.lifeTotalChanges.push_back(LifeTotalChange{player->id, lifeTotal, lifeTotal - (int)dtt.amount});
#ifdef DEBUG
			std::cout << dtt.amount << " dealt to player " << dtt.target << std::endl;
#endif
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
#ifdef DEBUG
		std::cout << (tt.tap ? "Tapping " : "Untapping ") << pObject->name << std::endl;
#endif
		pObject->isTapped = tt.tap;
	}
	for (CreateTargets& ct : changeset.target) {
		this->env.targets[ct.object] = ct.targets;
	}
    if(changeset.phaseChange.changed){
		changeset.phaseChange.starting = this->env.currentPhase;
		// CodeReview: How to handle extra steps?
		// CodeReview: Handle mana that doesn't empty
		// 500.4. When a step or phase ends, any unused mana left in a player's mana pool empties. This turn-based
		// action doesn't use the stack.
#ifdef DEBUG
		std::cout << "Clearing mana pools" << std::endl;
#endif
		for (auto& manaPool : this->env.manaPools) {
			manaPool.second.clear();
		}
		
		// CodeReview: For now have this happen here, not generally safe since multiple cleanup steps can happen
		// Eventually should get moved to happen after moving into an Untap step
		if(this->env.currentPhase == CLEANUP) {
			// CodeReview: Implement Phasing
			// 502.1. First, all phased-in permanents with phasing that the active player controls phase out, and all
			// phased-out permanents that the active player controlled when they phased out phase in. This all happens
			// simultaneously. This turn-based action doesn't use the stack.
			// CodeReview: Get next player from Environment
            unsigned int nextPlayer = ( this->env.turnPlayer + 1 ) % this->env.players.size();
			this->env.currentPlayer = nextPlayer;
            this->env.turnPlayer = nextPlayer;
            this->env.currentPhase = UNTAP;
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			this->env.landPlays[turnPlayerId] = 0;
			
			// 502.2. Second, the active player determines which permanents they control will untap. Then they untap
			// them all simultaneously. This turn-based action doesn't use the stack. Normally, all of a player's
			// permanents untap, but effects can keep one or more of a player's permanents from untapping.

			// CodeReview: Implemented restricted untap system
			// Get a set of filters/restrictions(card -> vector<card> -> bool) that says whether it can untap given all the other things that are untapping
			// Get the set of all remaining cards that could untap given what's already untapping
			// Have the strategy pick one to untap
			// Repeat until remaining cards to untap is empty
			Changeset untap;
			untap.tap.reserve(this->env.battlefield->objects.size());
			for (auto& object : this->env.battlefield->objects) {
				std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(object);
				if (this->env.getController(card->id) == turnPlayerId && card->isTapped) {
					untap.tap.push_back(TapTarget{ card->id, false });
				}
			}

			// 502.3.No player receives priority during the untap step, so no spells can be cast or resolve and no
			// abilities can be activated or resolve.Any ability that triggers during this step will be held until
			// the next time a player would receive priority, which is usually during the upkeep step.
			untap.phaseChange = StepOrPhaseChange{ true, UNTAP };
#ifdef DEBUG
			std::cout << "Untapping permanents for " << turnPlayerId << std::endl;
#endif
			this->applyChangeset(untap);
        }
		else if (this->env.currentPhase == ENDCOMBAT) {
			this->env.currentPhase = POSTCOMBATMAIN;
			this->env.declaredAttacks.clear();
			this->env.declaredBlocks.clear();
			this->env.blocked.clear();
			this->env.blockingOrder.clear();
		}
        else{
            this->env.currentPhase = (StepOrPhase)((int)this->env.currentPhase + 1);
			this->env.currentPlayer = this->env.turnPlayer;
        }
#ifdef DEBUG
		std::cout << "Moving to " << this->env.currentPhase << std::endl;
#endif

		if (this->env.currentPhase == DRAW) {
			// 504.1. First, the active player draws a card. This turn-based action doesn't use the stack.
			// CodeReview: Handle first turn don't draw
			Changeset drawCard = Changeset::drawCards(this->env.players[this->env.turnPlayer]->id, 1, env);
#ifdef DEBUG
			if(!drawCard.moves.empty())
				std::cout << this->env.players[this->env.turnPlayer]->id << "is drawing " << std::dynamic_pointer_cast<CardToken>(this->env.gameObjects.at(drawCard.moves[0].object))->name << std::endl;
#endif
			this->applyChangeset(drawCard);
		}
		else if (this->env.currentPhase == PRECOMBATMAIN) {
			// CodeReview: If archenemy do a scheme
			// 505.3. First, but only if the players are playing an Archenemy game (see rule 904), the active player is
			// the archenemy, and it's the active player's precombat main phase, the active player sets the top card of
			// their scheme deck in motion (see rule 701.24). This turn-based action doesn't use the stack.
			// CodeReview: Implement Saga's turn based action
			// 505.4. Second, if the active player controls one or more Saga enchantments and it's the active player's
			// precombat main phase, the active player puts a lore counter on each Saga they control. (See rule 714, "Saga Cards.")
			// This turn-based action doesn't use the stack.
		}
		else if (this->env.currentPhase == BEGINCOMBAT) {
			// 507.1. First, if the game being played is a multiplayer game in which the active player's opponents don't
			// all automatically become defending players, the active player chooses one of their opponents. That player
			// becomes the defending player. This turn-based action doesn't use the stack.
			// CodeReview: Appoint defending. Should always be automatic
		}
		else if (this->env.currentPhase == DECLAREATTACKERS) {
			std::set<xg::Guid> opponentsAndPlaneswalkers;
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			Player& player = *this->env.players[this->env.turnPlayer];
			for (auto& player : this->env.players) {
				if (player->id != turnPlayerId) opponentsAndPlaneswalkers.insert(player->id);
				for (auto& card : this->env.battlefield->objects) {
					std::shared_ptr<const CardToken> c = getBaseClassPtr<const CardToken>(card);
					if (env.getController(c) == turnPlayerId) continue;
					std::shared_ptr<const std::set<CardType>> types = this->env.getTypes(c);
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
				for (auto& card : this->env.battlefield->objects) {
					std::shared_ptr<CardToken> c = std::dynamic_pointer_cast<CardToken>(this->env.gameObjects[getBaseClassPtr<const Targetable>(card)->id]);
					if (declaredAttackers.find(c->id) != declaredAttackers.end()) continue;
					if (env.getController(c) != turnPlayerId) continue;
					if (!env.canAttack(c)) continue;
					possibleAttacks[c->id] = opponentsAndPlaneswalkers;
					// We ignore cost based restrictions for now. Up in the air how to decide them.
					auto restrictions = this->env.getAttackRestrictions(c);
					// CodeReview: If any restriction is CantAttackAloneRestriction and there is another creature that can attack or has the same restriction make an entry for both 
					for(auto& restriction : restrictions) {
					    possibleAttacks[c->id] = restriction.canAttack(c, possibleAttacks[c->id], declaredAttacks, env);
					}
					if(possibleAttacks[c->id].empty()) continue;
					requiredAttacks[c->id] = {};
					auto requirements = this->env.getAttackRequirements(c);
					for(auto& requirement : requirements) {
					    auto attacks = requirement->getRequiredAttacks(c, possibleAttacks[c->id], declaredAttacks, env);
					    requiredAttacks[c->id].insert(attacks.begin(), attacks.end());
					}
					if(!requiredAttacks[c->id].empty()) {
					    std::set<std::pair<size_t, xg::Guid>> opponentRequirements;
					    for(const xg::Guid& guid : possibleAttacks[c->id]) {
					        opponentRequirements.insert(std::make_pair(requiredAttacks[c->id].count(guid), guid));
					    }
					    std::set<xg::Guid> validAttacks;
					    size_t max = opponentRequirements.rbegin()->first;
					    for(auto iter=opponentRequirements.rbegin(); iter != opponentRequirements.rend(); iter++) {
					        if(iter->first != max) break;
					        validAttacks.insert(iter->second);
					    }
					    possibleAttacks[c->id] = validAttacks;
					}
					possibleAttackers.push_back(c);
				}
				// CodeReview: Need to figure out how to deal with costs to attack
				std::optional<std::pair<std::shared_ptr<CardToken>, xg::Guid>> attacker = player.strategy->chooseAttacker(possibleAttackers, possibleAttacks, requiredAttacks, declaredAttacks);
				if (attacker) {
					declaredAttacks.push_back(attacker.value());
					declaredAttackers.insert(attacker.value().first->id);
					declareAttacks.attacks.push_back(DeclareAttack{ attacker.value().first->id, attacker.value().second });
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
				this->applyChangeset(declareAttacks);
#ifdef DEBUG
				std::cout << turnPlayerId << " is declaring attacks" << std::endl;
				std::cout << declareAttacks << std::endl;
#endif
			}
			this->env.declaredAttacks = declaredAttacks;
			// CodeReview: Queue an event for declaring attackers
			// CodeReview: If there are no attackers skip to end of combat
		}
		else if (this->env.currentPhase == DECLAREBLOCKERS) {
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			for (auto& player : this->env.players) {
				if (player->id == turnPlayerId) continue;
				std::set<xg::Guid> attackers;
				for (auto& attacker : this->env.declaredAttacks) {
					if (attacker.second == player->id || this->env.getController(attacker.second) == player->id)
						attackers.insert(attacker.first->id);
				}
				std::vector<std::pair<std::shared_ptr<CardToken>, xg::Guid>> declaredBlocks;
				std::set<xg::Guid> declaredBlockers;
				do {
					std::vector<std::shared_ptr<CardToken>> possibleBlockers;
					std::map<xg::Guid, std::set<xg::Guid>> possibleBlocks;
					std::map<xg::Guid, std::multiset<xg::Guid>> requiredBlocks;
					for (auto& card : this->env.battlefield->objects) {
						std::shared_ptr<CardToken> c = std::dynamic_pointer_cast<CardToken>(this->env.gameObjects[getBaseClassPtr<const Targetable>(card)->id]);
						// CodeReview: Handle multiBlocking
						if (declaredBlockers.find(c->id) != declaredBlockers.end()) continue;
						if (env.getController(c) != player->id) continue;
						if (!env.canBlock(c)) continue;
						possibleBlocks[c->id] = attackers;
						// We ignore cost based restrictions for now. Up in the air how to decide them.
						auto restrictions = this->env.getBlockRestrictions(c);
						// If any restriction is CantBlockAloneRestriction and there is another creature that has the same restriction make an entry for both 
						for(auto& restriction : restrictions) {
						    possibleBlocks[c->id] = restriction->canBlock(c, possibleBlocks[c->id], declaredBlocks, env);
						}
						if(possibleBlocks[c->id].empty()) continue;
						requiredBlocks[c->id] = {};
						auto requirements = this->env.getBlockRequirements(c);
						for(auto& requirement : requirements) {
						    auto blocks = requirement->getRequiredBlocks(c, possibleBlocks[c->id], declaredBlocks, env);
						    requiredBlocks[c->id].insert(blocks.begin(), blocks.end());
						}
						if(!requiredBlocks[c->id].empty()) {
						    std::set<std::pair<size_t, xg::Guid>> blockersRequirements;
						    for(const xg::Guid& guid : possibleBlocks[c->id]) {
						        blockersRequirements.insert(std::make_pair(requiredBlocks[c->id].count(guid), guid));
						    }
						    std::set<xg::Guid> validBlocks;
						    size_t max = blockersRequirements.rbegin()->first;
						    for(auto iter=blockersRequirements.rbegin(); iter != blockersRequirements.rend(); iter++) {
						        if(iter->first != max) break;
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
						this->env.blocked.insert(blocker.value().second);
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

				this->env.declaredBlocks.insert(this->env.declaredBlocks.end(), declaredBlocks.begin(), declaredBlocks.end());
				// CodeReview: Queue an event for declaring blockers
				// CodeReview: Tell the environment that creatures which have a blocker assigned to them are blocked
			}
			for (auto& attacker : this->env.declaredAttacks) {
				std::vector<std::shared_ptr<CardToken>> blockedBy;
				for (auto& blocker : this->env.declaredBlocks) {
					if (blocker.second == attacker.first->id) blockedBy.push_back(blocker.first);
				}
				if (blockedBy.empty()) continue;
				Player& player = *std::dynamic_pointer_cast<Player>(this->env.gameObjects[this->env.getController(attacker.first)]);
				this->env.blockingOrder[attacker.first->id] = player.strategy->chooseBlockingOrder(attacker.first, blockedBy, env);
			}
			// CodeReview: For each blocking creature its controller chooses damage assignment order if multiple blockees
			// CodeReview: If any attacking or blocking creatures have first or double strike create a FirstStrikeDamageStep
		}
		else if (this->env.currentPhase == FIRSTSTRIKEDAMAGE) {
			// CodeReview: For this phase only consider creatures with first or double strike
			// Copy COMBATDAMAGE code
			// CodeReview: Mark all creatures that dealt damage as having dealt damage this combat
		}
		else if (this->env.currentPhase == COMBATDAMAGE) {
			Changeset damageEvent;
			if (!this->env.declaredAttacks.empty()) {
				for (auto& attack : this->env.declaredAttacks) {
					if (this->env.blocked.find(attack.first->id) != this->env.blocked.end()) {
						Player& player = *std::dynamic_pointer_cast<Player>(this->env.gameObjects[this->env.getController(attack.first)]);
						int powerRemaining = this->env.getPower(attack.first);
						for (auto& blocker : this->env.blockingOrder[attack.first->id]) {
							int minDamage = std::min(this->env.getLethalDamage(attack.first, blocker), powerRemaining);
							int damageAmount = player.strategy->chooseDamageAmount(attack.first, blocker, minDamage, powerRemaining, env);
							powerRemaining -= damageAmount;
							damageEvent.damage.push_back(DamageToTarget{ blocker, damageAmount });
							damageEvent.damage.push_back(DamageToTarget{ attack.first->id, this->env.getPower(blocker) });
						}
					}
					else {
						damageEvent.damage.push_back(DamageToTarget{ attack.second, this->env.getPower(attack.first) });
					}
				}
#ifdef DEBUG
				std::cout << "Applying combat damage" << std::endl;
#endif
				this->applyChangeset(damageEvent);
			}
			// CodeReview:For this phase do not consider creatures with first strike that dealt damage already this combat
			// CodeReview: If multiple attackers(similar for blockers) would damage the same creature the total has to be lethal to continue in blocking order but not the individual parcels
		}
		else if (this->env.currentPhase == CLEANUP) {
			xg::Guid turnPlayerId = this->env.players[this->env.turnPlayer]->id;
			this->env.currentPhase = CLEANUP;
			auto handObjects = this->env.hands.at(turnPlayerId)->objects;
			// CodeReview: Do this with Hand Size from StateQuery
			// 514.1. First, if the active player's hand contains more cards than their maximum hand size (normally
			// seven), they discard enough cards to reduce their hand size to that number. This turn-based action doesn't
			// use the stack.
			if (handObjects.size() > 7) {
				Changeset discard = Changeset::discardCards(turnPlayerId, handObjects.size() - 7, env);
#ifdef DEBUG
				std::cout << turnPlayerId << " is discarding " << handObjects.size() - 7 << " cards" << std::endl;
#endif
				this->applyChangeset(discard);
			}

			// 514.2. Second, the following actions happen simultaneously : all damage marked on permanents(including
			// phased - out permanents) is removed and all "until end of turn" and "this turn" effects end.This turn
			// - based action doesn't use the stack.
			// CodeReview: Send an end turn event through the system to get end of turn abilities to cleanup and any
			// relevant triggers
			// Should also move this into that changeset
			this->env.damage.clear();

			// 514.3a. At this point, the game checks to see if any state-based actions would be performed and/or any
			// triggered abilities are waiting to be put onto the stack (including those that trigger "at the beginning
			// of the next cleanup step"). If so, those state-based actions are performed, then those triggered abilities
			// are put on the stack, then the active player gets priority. Players may cast spells and activate abilities.
			// Once the stack is empty and all players pass in succession, another cleanup step begins.
			bool repeat = false;
			std::variant<std::monostate, Changeset> sba = this->checkStateBasedActions();
			while (Changeset* pChangeset = std::get_if<Changeset>(&sba)) {
				repeat = true;
				this->applyChangeset(*pChangeset);
				sba = this->checkStateBasedActions();
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
			cleanup.phaseChange = StepOrPhaseChange{ true, CLEANUP };
			this->applyChangeset(cleanup);
			// }
		}
	}
	if (changeset.clearTriggers) {
		this->env.triggers.clear();
	}
	for (QueueTrigger& qt : changeset.trigger) {
		this->env.triggers.push_back(qt);
	}
	for (LandPlay& pl : changeset.land) {
		env.landPlays[pl.player] += 1;
#ifdef DEBUG
		std::cout << pl.player << " is playing a land: " << std::dynamic_pointer_cast<CardToken>(this->env.gameObjects.at(pl.land))->name
				  << ". They've played " << env.landPlays[pl.player] << " lands this turn" << std::endl;
#endif
		Changeset moveLand;
		moveLand.moves.push_back(ObjectMovement{ pl.land, pl.zone, this->env.battlefield->id });
		this->applyChangeset(moveLand);
	}
	for (std::shared_ptr<ManaAbility> ma : changeset.manaAbility) {
#ifdef DEBUG
		std::cout << "Applying mana ability of " << getBaseClassPtr<const CardToken>(ma->source)->name << std::endl;
#endif
		Changeset apply = ma->applyEffect(env);
		this->applyChangeset(apply);
	}
    for(ObjectMovement& om : changeset.moves) {
		std::shared_ptr<Targetable> object = this->env.gameObjects.at(om.object);
		Changeset remove;
		remove.remove.push_back(RemoveObject{ om.object, om.sourceZone });
		this->applyChangeset(remove);
		Changeset create;
		object->id = om.newObject;
		create.create.push_back(ObjectCreation{ om.destinationZone, object });
		this->applyChangeset(create);
	}
	for (xg::Guid& ltg : changeset.loseTheGame) {
#ifdef DEBUG
		std::cout << ltg << " loses the game" << std::endl;
#endif
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
					if (getBaseClassPtr<const Targetable>(card)->owner == ltg) {
						removeCards.remove.push_back(RemoveObject{ getBaseClassPtr<const Targetable>(card)->id, this->env.battlefield->id });
					}
				}
				applyChangeset(removeCards);
				env.players.erase(iter);
				break;
			}
			index++;
		}
	}

	bool apply = false;
	Changeset triggers;
	for (std::shared_ptr<TriggerHandler> eh : this->env.triggerHandlers) {
		auto changePrelim = eh->handleEvent(changeset, this->env);
		if (std::vector<Changeset>* pChangeset = std::get_if<std::vector<Changeset>>(&changePrelim)) {
			std::vector<Changeset>& changes = *pChangeset;
			for (Changeset& change : changes) {
				triggers += change;
#ifdef DEBUG
				// CodeReview: This is not safe because of emblems
				std::cout << "Triggering an ability of " << std::dynamic_pointer_cast<CardToken>(this->env.gameObjects.at(eh->owner))->name << std::endl;
#endif
			}
			apply = true;
		}
	}
	if (apply) this->applyChangeset(triggers);

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