#include <algorithm>
#include <random>

#include "environment.h"
#include "effects/rulesEffects.h"

Environment::Environment(const std::vector<Player>& prelimPlayers, const std::vector<std::vector<Card>>& libraries)
: battlefield(new Zone<Card, Token>(BATTLEFIELD)), stack(new Zone<Card, Token, Ability>(STACK)),
  exile(new Zone<Card, Token>(EXILE)), command(new Zone<Card, Emblem>(COMMAND)), currentPhase(UPKEEP),
  currentPlayer(0), turnPlayer(0)
{
	this->changes.reserve(256);
	// CodeReview: Holding pointers to class members causes crashes on destruction
	this->gameObjects[battlefield->id] = battlefield;
	this->gameObjects[stack->id] = stack;
	this->gameObjects[exile->id] = exile;
	this->gameObjects[command->id] = command;
	for (const Player& player : prelimPlayers) {
		players.push_back(std::shared_ptr<Player>(new Player(player)));
	}
    for(unsigned int i=0; i < players.size(); i++) {
        this->gameObjects[players[i]->id] = players[i];
        this->hands[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(HAND));
		this->gameObjects[this->hands[players[i]->id]->id] = std::dynamic_pointer_cast<Targetable>(this->hands[players[i]->id]);
		this->libraries[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(LIBRARY));
		this->gameObjects[this->libraries[players[i]->id]->id] = std::dynamic_pointer_cast<Targetable>(this->libraries[players[i]->id]);
		this->graveyards[players[i]->id] = std::shared_ptr<Zone<Card, Token>>(new Zone<Card, Token>(GRAVEYARD));
		this->gameObjects[this->graveyards[players[i]->id]->id] = std::dynamic_pointer_cast<Targetable>(this->graveyards[players[i]->id]);

		this->landPlays[players[i]->id] = 0;
		this->lifeTotals[players[i]->id] = 20;
		this->manaPools[players[i]->id] = Mana();
		this->playerCounters[players[i]->id] = { {POISONCOUNTER, 0} };
        for(const Card& card : libraries[i]) {
			std::shared_ptr<Targetable> copy(new Card(card));
			copy->owner = players[i]->id;
			copy->id = xg::newGuid();
			std::dynamic_pointer_cast<Card>(copy)->source = std::dynamic_pointer_cast<Card>(copy);
			this->libraries[players[i]->id]->addObject(copy);
            this->gameObjects[copy->id] = copy;
			std::vector<std::shared_ptr<EventHandler>> replacement = this->getReplacementEffects(std::dynamic_pointer_cast<CardToken>(copy), LIBRARY);
			this->replacementEffects.insert(this->replacementEffects.end(), replacement.begin(), replacement.end());
			for (auto& r : replacement) r->owner = copy->id;
			std::vector<std::shared_ptr<TriggerHandler>> trigger = this->getTriggerEffects(std::dynamic_pointer_cast<CardToken>(copy), LIBRARY);
			this->triggerHandlers.insert(this->triggerHandlers.end(), trigger.begin(), trigger.end());
			for (auto& t : trigger) t->owner = copy->id;
			std::vector<std::shared_ptr<StaticEffectHandler>> state = this->getStaticEffects(std::dynamic_pointer_cast<CardToken>(copy), LIBRARY);
			this->stateQueryHandlers.insert(this->stateQueryHandlers.end(), state.begin(), state.end());
			for (auto& s : state) s->owner = copy->id;
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

int Environment::getPower(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getPower(card);
}

int Environment::getPower(std::shared_ptr<const CardToken> target) const{
	PowerQuery query{ *target, target->basePower };
	return std::get<PowerQuery>(executeStateQuery(query)).currentValue;
}

int Environment::getToughness(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getToughness(card);
}

int Environment::getToughness(std::shared_ptr<const CardToken> target) const {
	ToughnessQuery query{ *target, target->baseToughness };
	return std::get<ToughnessQuery>(executeStateQuery(query)).currentValue;
}

bool Environment::goodTiming(xg::Guid target) const {
	std::shared_ptr<CostedEffect> effect = std::dynamic_pointer_cast<CostedEffect>(gameObjects.at(target));
	return this->goodTiming(effect);
}

bool Environment::goodTiming(std::shared_ptr<const CostedEffect> target) const {
	bool value = false;
	if (std::shared_ptr<const Card> card = std::dynamic_pointer_cast<const Card>(target)) {
		std::shared_ptr<const std::set<CardType>> types = this->getTypes(std::dynamic_pointer_cast<const CardToken>(card));
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

std::shared_ptr<const std::set<CardSuperType>> Environment::getSuperTypes(std::shared_ptr<const CardToken> target)  const {
	SuperTypesQuery query{ *target, target->baseSuperTypes };
	return std::get<SuperTypesQuery>(executeStateQuery(query)).superTypes;
}

std::shared_ptr<const std::set<CardType>> Environment::getTypes(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getTypes(card);
}

std::shared_ptr<const std::set<CardType>> Environment::getTypes(std::shared_ptr<const CardToken> target) const {
	TypesQuery query{ *target, target->baseTypes };
	return std::get<TypesQuery>(executeStateQuery(query)).types;
}

std::shared_ptr<const std::set<CardSubType>> Environment::getSubTypes(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getSubTypes(card);
}

std::shared_ptr<const std::set<CardSubType>> Environment::getSubTypes(std::shared_ptr<const CardToken> target)  const {
	SubTypesQuery query{ *target, target->baseSubTypes };
	return std::get<SubTypesQuery>(executeStateQuery(query)).subTypes;
}

std::set<Color> Environment::getColors(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getColors(card);
}
std::set<Color> Environment::getColors(std::shared_ptr<const CardToken> target)  const {
	ColorsQuery query{ *target, target->baseColors };
	return std::get<ColorsQuery>(executeStateQuery(query)).colors;
}

xg::Guid Environment::getController(xg::Guid target) const {
	std::shared_ptr<Targetable> card = gameObjects.at(target);
	return this->getController(card);
}

xg::Guid Environment::getController(std::shared_ptr<const Targetable> target) const {
	ControllerQuery query{ *target, target->owner };
	return std::get<ControllerQuery>(executeStateQuery(query)).controller;
}

std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> Environment::getActivatedAbilities(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getActivatedAbilities(card);
}

std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> Environment::getActivatedAbilities(std::shared_ptr<const CardToken> target) const {
	ActivatedAbilitiesQuery query{ *target, target->activatableAbilities };
	return std::get<ActivatedAbilitiesQuery>(executeStateQuery(query)).abilities;
}
unsigned int Environment::getLandPlays(xg::Guid player) const {
	LandPlaysQuery query{ player, 1 };
	return std::get<LandPlaysQuery>(this->executeStateQuery(query)).amount;
}

std::vector<std::shared_ptr<EventHandler>> Environment::getReplacementEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<EventHandler>> handlers;
	for (const auto& h : target->replacementEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
				|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	ReplacementEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<ReplacementEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<TriggerHandler>> Environment::getTriggerEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<TriggerHandler>> handlers;
	for (const auto& h : target->triggerEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	TriggerEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<TriggerEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<StaticEffectHandler>> Environment::getStaticEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<StaticEffectHandler>> handlers;
	for (const auto& h : target->staticEffects) {
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	StaticEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<StaticEffectsQuery>(this->executeStateQuery(query)).effects;
}

std::vector<std::shared_ptr<EventHandler>> Environment::getSelfReplacementEffects(std::shared_ptr<const HasAbilities> target, ZoneType destinationZone, std::optional<ZoneType> sourceZone) const {
	std::vector<std::shared_ptr<EventHandler>> handlers;
	for (const size_t& i : target->thisOnlyReplacementIndexes) {
		const auto& h = target->replacementEffects[i];
		if ((sourceZone && h->activeSourceZones.find(sourceZone.value()) != h->activeSourceZones.end())
			|| h->activeDestinationZones.find(destinationZone) != h->activeDestinationZones.end())
			handlers.push_back(h);
	}
	SelfReplacementEffectsQuery query{ *target, destinationZone, sourceZone, handlers };
	return std::get<SelfReplacementEffectsQuery>(this->executeStateQuery(query)).effects;
}

StaticEffectQuery& Environment::executeStateQuery(StaticEffectQuery&& query) const {
	for (std::shared_ptr<StaticEffectHandler> sqh : this->stateQueryHandlers) {
		sqh->handleEvent(query, *this);
	}
	return query;
}

bool Environment::canAttack(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->canAttack(card);
}

bool Environment::canAttack(std::shared_ptr<const CardToken> target) const {
	std::shared_ptr<const std::set<CardType>> types = this->getTypes(target);
	if (types->find(CREATURE) == types->end()) return false;
	CanAttackQuery query{ *target, !target->isSummoningSick };
	return std::get<CanAttackQuery>(executeStateQuery(query)).canAttack;
}

bool Environment::canBlock(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->canBlock(card);
}

bool Environment::canBlock(std::shared_ptr<const CardToken> target) const {
	std::shared_ptr<const std::set<CardType>> types = this->getTypes(target);
	if (types->find(CREATURE) == types->end()) return false;
	CanBlockQuery query{ *target, !target->isTapped };
	return std::get<CanBlockQuery>(executeStateQuery(query)).canBlock;
}

int Environment::getLethalDamage(xg::Guid attacker, xg::Guid blocker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(attacker));
	return this->getLethalDamage(card, blocker);
}

int Environment::getLethalDamage(std::shared_ptr<const CardToken> attacker, xg::Guid blocker)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(blocker));
	return this->getLethalDamage(attacker, card);
}

int Environment::getLethalDamage(std::shared_ptr<const CardToken> attacker, std::shared_ptr<const CardToken> blocker) const {
	LethalDamageQuery query{ *attacker, *blocker, this->getToughness(blocker) - tryAtMap(this->damage, blocker->id, 0) };
	return std::get<LethalDamageQuery>(executeStateQuery(query)).damage;
}