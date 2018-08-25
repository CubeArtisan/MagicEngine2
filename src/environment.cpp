#include <algorithm>
#include <random>

#include "ability.h"
#include "environment.h"
#include "rulesEffects.h"

Environment::Environment(const std::vector<Player>& prelimPlayers, const std::vector<std::vector<Card>>& libraries)
: battlefield(new Zone<Card, Token>(BATTLEFIELD)), stack(new Zone<Card, Token, Ability>(STACK)),
  exile(new Zone<Card, Token>(EXILE)), command(new Zone<Card, Emblem>(COMMAND)), currentPhase(UPKEEP),
  currentPlayer(0), turnPlayer(0)
{
	this->changes.reserve(1024);
	// CodeReview: Holding pointers to class members causes crashes on destruction
	this->gameObjects[battlefield->id] = battlefield;
	battlefield->objects.reserve(128);
	this->gameObjects[stack->id] = stack;
	stack->objects.reserve(16);
	this->gameObjects[exile->id] = exile;
	exile->objects.reserve(32);
	this->gameObjects[command->id] = command;
	command->objects.reserve(8);
	for (const Player& player : prelimPlayers) {
		players.push_back(player.clone());
	}
    for(unsigned int i=0; i < players.size(); i++) {
        this->gameObjects[players[i]->id] = players[i];
        this->hands[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(HAND));
		this->hands[players[i]->id]->objects.reserve(8);
		this->gameObjects[this->hands[players[i]->id]->id] = this->hands[players[i]->id];
		this->libraries[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(LIBRARY));
		this->libraries[players[i]->id]->objects.reserve(128);
		this->gameObjects[this->libraries[players[i]->id]->id] = this->libraries[players[i]->id];
		this->graveyards[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(GRAVEYARD));
		this->graveyards[players[i]->id]->objects.reserve(128);
		this->gameObjects[this->graveyards[players[i]->id]->id] = this->graveyards[players[i]->id];

		this->landPlays[players[i]->id] = 0;
		this->lifeTotals[players[i]->id] = 20;
		this->manaPools[players[i]->id] = Mana();
		this->playerCounters[players[i]->id] = { {POISONCOUNTER, 0} };
        for(const Card& card : libraries[i]) {
			std::shared_ptr<Card> copy(card.clone());
			copy->owner = players[i]->id;
			copy->id = xg::newGuid();
			this->libraries[players[i]->id]->addObject(copy);
            this->gameObjects[copy->id] = copy;
			std::vector<std::shared_ptr<const EventHandler>> replacement = this->getReplacementEffects(copy, LIBRARY);
			std::vector<std::shared_ptr<const EventHandler>> replacements;
			replacements.reserve(replacement.size());
			for (auto& r : replacement) {
				std::shared_ptr<EventHandler> res = r->clone();
				res->owner = copy->id;
				replacements.push_back(res);
			}
			this->replacementEffects.insert(this->replacementEffects.end(), replacements.begin(), replacements.end());
			std::vector<std::shared_ptr<const TriggerHandler>> trigger = this->getTriggerEffects(copy, LIBRARY);
			std::vector<std::shared_ptr<const TriggerHandler>> triggers;
			triggers.reserve(trigger.size());
			for (auto& t : trigger) {
				std::shared_ptr<TriggerHandler> trig = t->clone();
				trig->owner = copy->id;
				triggers.push_back(trig);
			}
			this->triggerHandlers.insert(this->triggerHandlers.end(), triggers.begin(), triggers.end());
			std::vector<std::shared_ptr<const StaticEffectHandler>> state = this->getStaticEffects(copy, LIBRARY);
			std::vector<std::shared_ptr<const StaticEffectHandler>> states;
			states.reserve(state.size());
			for (auto& t : state) {
				std::shared_ptr<StaticEffectHandler> trig = t->clone();
				trig->owner = copy->id;
				states.push_back(trig);
			}
			this->stateQueryHandlers.insert(this->stateQueryHandlers.end(), state.begin(), state.end());
		}
        
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(this->libraries[players[i]->id]->objects.begin(), this->libraries[players[i]->id]->objects.end(), g);
	}
	this->createRulesEffects();
	// CodeReview: Ask all cards in all zones to register their handlers that would apply from that zone
}

void Environment::createRulesEffects() {
	this->replacementEffects.push_back(std::shared_ptr<EventHandler>(new TokenMovementEffect()));
	this->replacementEffects.push_back(std::shared_ptr<EventHandler>(new ZeroDamageEffect()));
	this->stateQueryHandlers.push_back(std::shared_ptr<StaticEffectHandler>(new CounterPowerToughnessEffect()));
}

template<typename T, typename... Extra, typename... Args>
void addObjectInternal(Zone<Args...>& zone, const std::shared_ptr<const Targetable>& object, int index) {
	if (std::shared_ptr<const T> result = std::dynamic_pointer_cast<const T>(object)) {
		if (index >= 0) zone.objects.insert(zone.objects.begin() + (zone.objects.size() - index), result);
		else zone.objects.insert(zone.objects.begin() + (1 - index), result);
	}
	else {
		if constexpr (sizeof...(Extra) == 0) {
#ifdef DEBUG
			std::cerr << "Could not convert to internal types" << std::endl;
#endif
			throw "Could not convert to internal types";
		}
		else {
			return addObjectInternal<Extra...>(zone, object, index);
		}
	}
}

int Environment::getPower(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getPower(card);
}

int Environment::getPower(const std::shared_ptr<const CardToken>& target) const{
	PowerQuery query{ *target, target->basePower };
	return std::get<PowerQuery>(executeStateQuery(query)).power;
}

int Environment::getToughness(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getToughness(card);
}

int Environment::getToughness(const std::shared_ptr<const CardToken>& target) const {
	ToughnessQuery query{ *target, target->baseToughness };
	return std::get<ToughnessQuery>(executeStateQuery(query)).toughness;
}

bool Environment::goodTiming(xg::Guid target) const {
	std::shared_ptr<HasCost> effect = std::dynamic_pointer_cast<HasCost>(gameObjects.at(target));
	return this->goodTiming(effect);
}

bool Environment::goodTiming(const std::shared_ptr<const HasCost>& target) const {
	bool value = false;
	if (std::shared_ptr<const Card> card = std::dynamic_pointer_cast<const Card>(target)) {
		std::shared_ptr<const std::set<CardType>> types = this->getTypes(card);
		if (types->find(INSTANT) != types->end()) value = true;
		else value = (this->currentPhase == PRECOMBATMAIN || this->currentPhase == POSTCOMBATMAIN)
			&& this->stack->objects.empty()
			&& this->players[this->turnPlayer]->id == card->owner;
	}
	// CodeReview: Handle abilities that are sorcery speed only
	else value = true;
	TimingQuery query{ *target, value };
	return std::get<TimingQuery>(executeStateQuery(query)).timing;
}

std::shared_ptr<const std::set<CardSuperType>> Environment::getSuperTypes(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getSuperTypes(card);
}

std::shared_ptr<const std::set<CardSuperType>> Environment::getSuperTypes(const std::shared_ptr<const CardToken>& target)  const {
	SuperTypesQuery query{ *target, target->baseSuperTypes };
	return std::get<SuperTypesQuery>(executeStateQuery(query)).superTypes;
}

std::shared_ptr<const std::set<CardType>> Environment::getTypes(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getTypes(card);
}

std::shared_ptr<const std::set<CardType>> Environment::getTypes(const std::shared_ptr<const CardToken>& target) const {
	TypesQuery query{ *target, target->baseTypes };
	return std::get<TypesQuery>(executeStateQuery(query)).types;
}

std::shared_ptr<const std::set<CardSubType>> Environment::getSubTypes(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getSubTypes(card);
}

std::shared_ptr<const std::set<CardSubType>> Environment::getSubTypes(const std::shared_ptr<const CardToken>& target)  const {
	SubTypesQuery query{ *target, target->baseSubTypes };
	return std::get<SubTypesQuery>(executeStateQuery(query)).subTypes;
}

std::set<Color> Environment::getColors(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getColors(card);
}
std::set<Color> Environment::getColors(const std::shared_ptr<const CardToken>& target)  const {
	ColorsQuery query{ *target, target->baseColors };
	return std::get<ColorsQuery>(executeStateQuery(query)).colors;
}

xg::Guid Environment::getController(xg::Guid target) const {
	std::shared_ptr<Targetable> card = gameObjects.at(target);
	return this->getController(card);
}

xg::Guid Environment::getController(const std::shared_ptr<const Targetable>& target) const {
	ControllerQuery query{ *target, target->owner };
	return std::get<ControllerQuery>(executeStateQuery(query)).controller;
}

std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> Environment::getActivatedAbilities(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getActivatedAbilities(card);
}

std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> Environment::getActivatedAbilities(const std::shared_ptr<const CardToken>& target) const {
	ActivatedAbilitiesQuery query{ *target, target->activatableAbilities };
	return std::get<ActivatedAbilitiesQuery>(executeStateQuery(query)).abilities;
}
unsigned int Environment::getLandPlays(xg::Guid player) const {
	LandPlaysQuery query{ player, 1 };
	return std::get<LandPlaysQuery>(this->executeStateQuery(query)).amount;
}

std::vector<std::shared_ptr<const EventHandler>> Environment::getReplacementEffects(const std::shared_ptr<const CardToken>& target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<const EventHandler>> handlers;
	for (const auto& h : target->replacementEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
				|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	ReplacementEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<ReplacementEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const EventHandler>> Environment::getActiveReplacementEffects() const
{
	ActiveReplacementEffectsQuery query{ this->replacementEffects };
	return std::get<ActiveReplacementEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const TriggerHandler>> Environment::getTriggerEffects(const std::shared_ptr<const CardToken>& target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<const TriggerHandler>> handlers;
	for (const auto& h : target->triggerEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	TriggerEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<TriggerEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const TriggerHandler>> Environment::getActiveTriggerEffects() const
{
	ActiveTriggerEffectsQuery query{ this->triggerHandlers };
	return std::get<ActiveTriggerEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const StaticEffectHandler>> Environment::getStaticEffects(const std::shared_ptr<const CardToken>& target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<const StaticEffectHandler>> handlers;
	for (const auto& h : target->staticEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	StaticEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<StaticEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const StaticEffectHandler>> Environment::getActiveStaticEffects() const {
	ActiveStaticEffectsQuery query{ this->stateQueryHandlers };
	return std::get<ActiveStaticEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<const EventHandler>> Environment::getSelfReplacementEffects(const std::shared_ptr<const CardToken>& target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<const EventHandler>> handlers;
	for (const size_t& i : target->thisOnlyReplacementIndexes) {
		const auto& h = target->replacementEffects[i];
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	SelfReplacementEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<SelfReplacementEffectsQuery>(this->executeStateQuery(query)).effects;
}

bool Environment::canAttack(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->canAttack(card);
}

bool Environment::canAttack(const std::shared_ptr<const CardToken>& target) const {
	std::shared_ptr<const std::set<CardType>> types = this->getTypes(target);
	if (types->find(CREATURE) == types->end()) return false;
	CanAttackQuery query{ *target, !target->isSummoningSick };
	return std::get<CanAttackQuery>(executeStateQuery(query)).canAttack;
}

bool Environment::canBlock(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->canBlock(card);
}

bool Environment::canBlock(const std::shared_ptr<const CardToken>& target) const {
	std::shared_ptr<const std::set<CardType>> types = this->getTypes(target);
	if (types->find(CREATURE) == types->end()) return false;
	CanBlockQuery query{ *target, !target->isTapped };
	return std::get<CanBlockQuery>(executeStateQuery(query)).canBlock;
}

int Environment::getLethalDamage(xg::Guid attacker, xg::Guid blocker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(attacker));
	return this->getLethalDamage(card, blocker);
}

int Environment::getLethalDamage(const std::shared_ptr<const CardToken>& attacker, xg::Guid blocker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(blocker));
	return this->getLethalDamage(attacker, card);
}

int Environment::getLethalDamage(const std::shared_ptr<const CardToken>& attacker, const std::shared_ptr<const CardToken>& blocker) const {
	LethalDamageQuery query{ *attacker, *blocker, this->getToughness(blocker) - tryAtMap(this->damage, blocker->id, 0) };
	return std::get<LethalDamageQuery>(executeStateQuery(query)).damage;
}

std::vector<AttackRestrictionValue> Environment::getAttackRestrictions(xg::Guid attacker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(attacker));
	return this->getAttackRestrictions(card);
}

std::vector<AttackRestrictionValue> Environment::getAttackRestrictions(const std::shared_ptr<const CardToken>& attacker) const {
	AttackRestrictionQuery query{ *attacker, {} };
	return std::get<AttackRestrictionQuery>(executeStateQuery(query)).restrictions;
}

std::vector<AttackRequirementValue> Environment::getAttackRequirements(xg::Guid attacker) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(attacker));
	return this->getAttackRequirements(card);
}

std::vector<AttackRequirementValue> Environment::getAttackRequirements(const std::shared_ptr<const CardToken>& attacker) const {
	AttackRequirementQuery query{ *attacker,{} };
	return std::get<AttackRequirementQuery>(executeStateQuery(query)).requirements;
}

std::vector<BlockRestrictionValue> Environment::getBlockRestrictions(xg::Guid blocker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(blocker));
	return this->getBlockRestrictions(card);
}

std::vector<BlockRestrictionValue> Environment::getBlockRestrictions(const std::shared_ptr<const CardToken>& blocker) const {
	BlockRestrictionQuery query{ *blocker,{} };
	return std::get<BlockRestrictionQuery>(executeStateQuery(query)).restrictions;
}

std::vector<BlockRequirementValue> Environment::getBlockRequirements(xg::Guid blocker) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(blocker));
	return this->getBlockRequirements(card);
}

std::vector<BlockRequirementValue> Environment::getBlockRequirements(const std::shared_ptr<const CardToken>& blocker) const {
	BlockRequirementQuery query{ *blocker,{} };
	return std::get<BlockRequirementQuery>(executeStateQuery(query)).requirements;
}

StaticEffectQuery& Environment::executeStateQuery(StaticEffectQuery&& query) const {
	// CodeReview 107.1b If a calculation yields a negative value use 0 instead, with exceptions
	// CodeReview: 107.2 some calculations can't complete what should we do then
	// CodeReview: If layer 7 or nonexistent get static effects
	// Should order the indexes of StaticEffectQuery by layers to simplify this process
	std::vector<std::shared_ptr<const StaticEffectHandler>> effects;
	if (!std::holds_alternative<ActiveStaticEffectsQuery>(query)) {
		effects = this->getActiveStaticEffects();
	}
	else {
		effects = this->stateQueryHandlers;
	}
	for (std::shared_ptr<const StaticEffectHandler> sqh : effects) {
		sqh->handleEvent(query, *this);
	}
	return query;
}