#include "ability.h"
#include "card.h"
#include "guid.hpp"
#include "runner.h"
#include "targeting.h"

std::optional<Changeset> Runner::checkStateBasedActions() const {
	bool apply = false;
	Changeset stateBasedAction;
	for (const std::shared_ptr<Player>& player : this->env.players) {
		// 704.5a. If a player has 0 or less life, that player loses the game.
		if (this->env.lifeTotals.at(player->id) <= 0) {
			apply = true;
			stateBasedAction.push_back<LoseTheGame>(player->id);
		}
		// 704.5c. If a player has ten or more poison counters, that player loses the game.
		else if (this->env.playerCounters.at(player->id).at(POISONCOUNTER) >= 10) {
			apply = true;
			stateBasedAction.push_back<LoseTheGame>(player->id);
		}
	}

	// CodeReview: Technically milling out should happen here not in the draw function

	// 704.5d If a token is in a zone other than the battlefield, it ceases to exist
	// 704.5e. If a copy of a spell is in a zone other than the stack, it ceases to exist. If a copy of a card is in any zone other than the stack or the battlefield, it ceases to exist.
	// Ignores the stack since Tokens on the stack are assumed to be spell copies
	for (auto& pair : this->env.hands) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.push_back<RemoveObject>((*token)->id, pair.second->id);
			apply = true;
		}
	for (auto& pair : this->env.libraries) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& pair : this->env.graveyards) for (auto& variant : pair.second->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.push_back(RemoveObject{ (*token)->id, pair.second->id });
			apply = true;
		}
	for (auto& variant : this->env.exile->objects)
		if (std::shared_ptr<const Token>* token = std::get_if<std::shared_ptr<const Token>>(&variant)) {
			stateBasedAction.push_back<RemoveObject>((*token)->id, this->env.exile->id);
			apply = true;
		}

	for (auto& variant : this->env.battlefield->objects) {
		std::shared_ptr<const CardToken> card = getBaseClassPtr<const CardToken>(variant);
		std::shared_ptr<const std::set<CardType>> types = this->env.getTypes(card);
		if (types->find(CREATURE) != types->end()) {
			int toughness = this->env.getToughness(card);
			// 704.5f. If a creature has toughness 0 or less, it's put into its owner's graveyard. Regeneration can't replace this event.
			if (toughness <= 0) {
				stateBasedAction.push_back<ObjectMovement>(card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id);
				apply = true;
			}
			// 704.5g.If a creature has toughness greater than 0, and the total damage marked on it is greater than or equal to its toughness, that creature has been dealt lethal damage and is destroyed.Regeneration can replace this event.
			else if (toughness <= tryAtMap(this->env.damage, card->id, 0)) {
				// CodeReview: Make a destroy change
				stateBasedAction.push_back<ObjectMovement>(card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id, 0, DESTROY);
				apply = true;
			}
			// CodeReview: Deal with deathtouch
		}
		else {
			std::shared_ptr<const std::set<CardSubType>> subtypes = this->env.getSubTypes(card);
			// 704.5m. If an Aura is attached to an illegal object or player, or is not attached to an object or player, that Aura is put into its owner's graveyard.
			// CodeReview: Bestow
			if (subtypes->find(AURA) != subtypes->end()) {
				if (!card->targeting->validTargets(this->env.targets.at(card->id), *card, this->env)) {
					stateBasedAction.push_back<ObjectMovement>(card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id);
					apply = true;
				}
			}
		}
		if (types->find(PLANESWALKER) != types->end()) {
			// 704.5i. If a planeswalker has loyalty 0, it's put into its owner's graveyard.
			// CodeReview: Assumes all planeswalkers will have an entry for loyalty counters in the map
			if (this->env.permanentCounters.at(card->id).at(LOYALTY) == 0) {
				stateBasedAction.push_back<ObjectMovement>(card->id, this->env.battlefield->id, this->env.graveyards.at(card->owner)->id);
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
			stateBasedAction.push_back<AddPermanentCounter>(pair.first, PLUSONEPLUSONECOUNTER, -amount);
			stateBasedAction.push_back<AddPermanentCounter>(pair.first, MINUSONEMINUSONECOUNTER, -amount);
			apply = true;
		}
	}

	if (apply)  return stateBasedAction;
	return std::nullopt;
}

std::optional<Changeset> Runner::executeStep() const {
	// 116.5. Each time a player would get priority, the game first performs all applicable state-based actions as a
	// single event (see rule 704, "State-Based Actions"), then repeats this process until no state-based actions are
	// performed. Then triggered abilities are put on the stack (see rule 603, "Handling Triggered Abilities"). These
	// steps repeat in order until no further state-based actions are performed and no abilities trigger. Then the
	// player who would have received priority does so.
	std::optional<Changeset> actions = this->checkStateBasedActions();
	if (actions) {
		return *actions;
	}

	if (!this->env.triggers.empty()) {
		// CodeReview: APNAP order and choices to be made here
		Changeset applyTriggers;
		for (const QueueTrigger& trigger : this->env.triggers) {
			std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(this->env.gameObjects.at(trigger.player));
			std::shared_ptr<Ability> ability = trigger.ability->clone();
			ability->id = xg::newGuid();
			ability->owner = player->id;
			ability->source = convertToVariant<SourceType>(env.gameObjects.at(trigger.source));
			applyTriggers.push_back<CreateObject>(this->env.stack->id, ability);
			std::vector<xg::Guid> targets = player->strategy->chooseTargets(ability, *player, env);
			if (!targets.empty()) {
				applyTriggers.push_back<CreateTargets>(ability->id, targets);
			}
		}
		applyTriggers.push_back<ClearTriggers>();
		return applyTriggers;
	}

	Player& active = *this->env.players[this->env.currentPlayer];
	GameAction action = active.strategy->chooseGameAction(active, env);
	if (CastSpell* pCastSpell = std::get_if<CastSpell>(&action)) {
		Changeset castSpell;
		std::shared_ptr<Zone<Card, Token>> hand = this->env.hands.at(active.id);
		std::shared_ptr<ObjectMovement> move{ new ObjectMovement{ pCastSpell->spell, hand->id, this->env.stack->id } };
		castSpell.changes.push_back(move);
		std::shared_ptr<Card> spell = std::dynamic_pointer_cast<Card>(this->env.gameObjects.at(pCastSpell->spell));
#ifdef DEBUG
		std::cout << "Casting " << spell->name << std::endl;
#endif
		if (pCastSpell->targets.size() > 0) {
			castSpell.push_back<CreateTargets>(move->newObject, pCastSpell->targets);
		}
		castSpell += pCastSpell->cost.payCost(active, env, spell);
		for (const CostValue& c : pCastSpell->additionalCosts) {
			castSpell += c->payCost(active, env, spell);
		}
		// CodeReview: Use the chosen X value
		return castSpell;
	}

	if (PlayLand* pPlayLand = std::get_if<PlayLand>(&action)) {
		std::vector<Changeset> results;
		Changeset playLand;
		std::shared_ptr<Zone<Card, Token>> hand = this->env.hands.at(active.id);
		playLand.push_back<LandPlay>(pPlayLand->land, active.id, hand->id);
		return playLand;
	}

	if (ActivateAnAbility* pActivateAnAbility = std::get_if<ActivateAnAbility>(&action)) {
		Changeset activateAbility;
		std::shared_ptr<ActivatedAbility> result(pActivateAnAbility->ability);
		result->source = pActivateAnAbility->source;
		result->owner = active.id;
		result->id = xg::newGuid();
		if (std::shared_ptr<ManaAbility> manaAbility = std::dynamic_pointer_cast<ManaAbility>(result)) {
			activateAbility.push_back<ApplyManaAbility>(manaAbility);
		}
		else {
			activateAbility.push_back<CreateObject>(this->env.stack->id, result);
		}
		if (pActivateAnAbility->targets.size() > 0) {
			activateAbility.push_back<CreateTargets>(result->id, pActivateAnAbility->targets);
		}
		CostValue cost = pActivateAnAbility->cost;
		activateAbility += cost.payCost(active, env, result->source);
#ifdef DEBUG
		std::cout << active.id << " is activating an ability of " << getBaseClassPtr<const CardToken>(pActivateAnAbility->source)->name << std::endl;
#endif
		// CodeReview: Use the chosen X value
		return activateAbility;
	}

	if (std::get_if<PassPriority>(&action)) {
		return std::nullopt;
	}

	return Changeset();
}

void Runner::runGame() {
	this->env = Environment(players, libraries);
	Changeset startDraw;
	for (std::shared_ptr<Player> player : this->env.players) {
		startDraw += Changeset::drawCards(player->id, 7, this->env);
	}
	this->applyChangeset(startDraw);
	// CodeReview: Handle mulligans

	const int UNSET_PLAYER_PASS = -1;
	int firstPlayerToPass = UNSET_PLAYER_PASS;
	while (this->env.players.size() > 1) {
		std::optional<Changeset> step = this->executeStep();

		if (step) {
			this->applyChangeset(*step);
			firstPlayerToPass = UNSET_PLAYER_PASS;
		}
		else {
			int nextPlayer = (this->env.currentPlayer + 1) % this->env.players.size();
			if (firstPlayerToPass == nextPlayer) {
				auto stack = this->env.stack;
				// 500.2. A phase or step in which players receive priority ends when the stack is empty and all players
				// pass in succession.Simply having the stack become empty doesn't cause such a phase or step to end;
				// all players have to pass in succession with the stack empty. Because of this, each player gets a chance
				// to add new things to the stack before that phase or step ends.
				if (stack->objects.empty()) {
					Changeset passStep;
					passStep.push_back<StepOrPhaseChange>(this->env.currentPhase);
					this->applyChangeset(passStep);
				}
				else {
					std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>,
						std::shared_ptr<const Ability>> top = this->env.stack->objects.back();
					std::shared_ptr<HasEffect> hasEffect = getBaseClassPtr<const HasEffect>(top)->clone();
					std::vector<xg::Guid> targets;
					if (env.targets.find(hasEffect->id) != env.targets.end()) targets = env.targets.at(hasEffect->id);
					if (hasEffect->targeting->anyValidTarget(targets, *hasEffect, env)) {
						hasEffect->resetEffect();
						std::optional<Changeset> resolveAbility = hasEffect->getChangeset(env);
						while (resolveAbility) {
							this->applyChangeset(resolveAbility.value());
							resolveAbility = hasEffect->getChangeset(env);
						}
						Changeset resolveSpellAbility;
						if (const std::shared_ptr<const Card>* pCard = std::get_if<std::shared_ptr<const Card>>(&top)) {
							std::shared_ptr<const Card> card = *pCard;
							bool isPermanent = false;
							for (CardType type : *this->env.getTypes(card)) {
								if (PERMANENTBEGIN < type && type < PERMANENTEND) {
									std::shared_ptr<ObjectMovement> move{ new ObjectMovement{ card->id, stack->id, this->env.battlefield->id } };
									resolveSpellAbility.changes.push_back(move);
									isPermanent = true;
									std::shared_ptr<const std::set<CardSubType>> subtypes = env.getSubTypes(card);
									if (subtypes->find(AURA) != subtypes->end()) {
										Changeset applyTargets;
										applyTargets.push_back<CreateTargets>(move->newObject, env.targets.at(card->id));
										this->applyChangeset(applyTargets);
									}
									break;
								}
							}
							if (!isPermanent) {
								resolveSpellAbility.push_back<ObjectMovement>(card->id, stack->id, this->env.graveyards.at(card->owner)->id);
							}
						}
						else {
							xg::Guid id = getBaseClassPtr<const Targetable>(top)->id;
							resolveSpellAbility.push_back<RemoveObject>(id, stack->id);
						}
						applyChangeset(resolveSpellAbility);
					}
					else {
						Changeset countered;
						if (std::dynamic_pointer_cast<CardToken>(hasEffect)) {
							countered.push_back<ObjectMovement>(hasEffect->id, stack->id, env.graveyards.at(hasEffect->owner)->id);
						}
						else {
							countered.push_back<RemoveObject>(hasEffect->id, stack->id);
						}
						this->applyChangeset(countered);
					}
				}
				this->env.currentPlayer = this->env.turnPlayer;
			}
			else {
				if (firstPlayerToPass == UNSET_PLAYER_PASS) {
					firstPlayerToPass = this->env.currentPlayer;
				}
				this->env.currentPlayer = nextPlayer;
			}
		}
	}
	std::cout << "Turn number " << this->env.turnNumber << std::endl;
}

void Runner::applyMoveRules(Changeset& changeset) {
	std::vector<std::tuple<std::shared_ptr<CardToken>, ZoneType>> objects;
	for (const std::shared_ptr<CreateObject>& create : changeset.ofType<CreateObject>()) {
		if (std::shared_ptr<CardToken> object = std::dynamic_pointer_cast<CardToken>(create->created)) {
			std::shared_ptr<ZoneInterface> destination = std::dynamic_pointer_cast<ZoneInterface>(env.gameObjects.at(create->zone));
			objects.push_back(std::make_tuple(object, destination->type));
		}
	}
	std::vector<Changeset> result;
	bool applyStatic = false;
	bool applyReplacement = false;
	Changeset addStatic;
	Changeset addReplacement;
	for (auto& object : objects) {
		std::vector<std::shared_ptr<const StaticEffectHandler>> staticHandlers = env.getStaticEffects(std::get<0>(object), std::get<1>(object), std::nullopt);
		for (const auto& h : staticHandlers) {
			std::shared_ptr<StaticEffectHandler> hd = h->clone();
			hd->owner = std::get<0>(object)->id;
			addStatic.changes.push_back(std::shared_ptr<GameChange>(new AddStaticEffect{ hd }));
		}
		applyStatic = !staticHandlers.empty();
		if (std::get<1>(object) != BATTLEFIELD) continue;
		// 614.12. Some replacement effects modify how a permanent enters the battlefield. (See rules 614.1c-d.) Such effects may come from the permanent itself if they
		// affect only that permanent (as opposed to a general subset of permanents that includes it). They may also come from other sources. To determine which replacement
		// effects apply and how they apply, check the characteristics of the permanent as it would exist on the battlefield, taking into account replacement effects that
		// have already modified how it enters the battlefield (see rule 616.1), continuous effects from the permanent's own static abilities that would apply to it once it's
		// on the battlefield, and continuous effects that already exist and would apply to the permanent.
		std::vector<std::shared_ptr<const EventHandler>> replacementHandlers = env.getSelfReplacementEffects(std::get<0>(object), std::get<1>(object), std::nullopt);
		for (auto& h : replacementHandlers) {
			std::shared_ptr<EventHandler> hd = h->clone();
			hd->owner = std::get<0>(object)->id;
			addReplacement.changes.push_back(std::shared_ptr<GameChange>(new AddReplacementEffect{ hd }));
		}
		applyReplacement = !replacementHandlers.empty();
	}
	if (applyStatic) this->applyChangeset(addStatic);
	if (applyReplacement) this->applyChangeset(addReplacement);
}

bool Runner::applyReplacementEffects(Changeset& changeset, std::set<xg::Guid> applied) {
	// CodeReview: Allow strategy to specify order to evaluate in
	for (std::shared_ptr<const EventHandler> eh : this->env.getActiveReplacementEffects()) {
		if (applied.find(eh->id) != applied.end()) continue;
		auto result = eh->handleEvent(changeset, this->env);
		if (result) {
			std::vector<Changeset>& changes = *result;
			applied.insert(eh->id);
			for (Changeset& change : changes) {
				if (!this->applyReplacementEffects(change, applied)) this->applyChangeset(change, false);
			}
			return true;
		}
	}
	return false;
}

bool Runner::applyChangeset(Changeset& changeset, bool replacementEffects, bool triggerEffects) {
	this->applyMoveRules(changeset);
	if (replacementEffects && this->applyReplacementEffects(changeset)) return true;
#ifdef DEBUG
	// std::cout << changeset;
#endif

	bool result = true;
	for (std::shared_ptr<GameChange> change : changeset.changes) {
		result &= change->ApplyTo(env, *this);
#ifdef DEBUG
		std::cout << change->ToString(env) << std::endl;
#endif
	}

	if (triggerEffects) {
		bool apply = false;
		Changeset triggers;
		std::vector<std::shared_ptr<const TriggerHandler>> handlers = env.getActiveTriggerEffects();
		for (std::shared_ptr<const TriggerHandler> eh : handlers) {
			auto changePrelim = eh->handleEvent(changeset, this->env);
			if (changePrelim) {
				std::vector<Changeset>& changes = *changePrelim;
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
	}
	this->env.changes.emplace_back(changeset);
	return result;
}

Runner::Runner(std::vector<std::vector<Card>>& libraries, std::vector<Player> players)
	: libraries(libraries), players(players), env(players, libraries)
{
	if (players.size() != libraries.size()) {
#ifdef DEBUG
		std::cerr << "Not equal players and libraries" << std::endl;
#endif
		throw "Not equal players and libraries";
	}
}