#include <algorithm>
#include <random>

#include "environment.h"

Environment::Environment(std::vector<Player>& prelimPlayers, std::vector<std::vector<Card>>& libraries)
: battlefield(BATTLEFIELD), stack(STACK), exile(EXILE), command(COMMAND), players(prelimPlayers),
  currentPhase(UPKEEP), currentPlayer(0), turnPlayer(0)
{
	// CodeReview: Holding pointers to class members causes crashes on destruction
	this->gameObjects[battlefield.id] = std::shared_ptr<Targetable>(&battlefield);
	this->gameObjects[stack.id] = std::shared_ptr<Targetable>(&stack);
	this->gameObjects[exile.id] = std::shared_ptr<Targetable>(&exile);
	this->gameObjects[command.id] = std::shared_ptr<Targetable>(&command);
    for(unsigned int i=0; i < players.size(); i++) {
        this->gameObjects[players[i].id] = std::shared_ptr<Targetable>(&players[0]);
        this->hands[players[i].id] = Zone<Card, Token>(HAND);
		this->gameObjects[this->hands[players[i].id].id] = std::shared_ptr<Targetable>(&this->hands[players[i].id]);
        this->libraries[players[i].id] = Zone<Card, Token>(LIBRARY);
		this->gameObjects[this->libraries[players[i].id].id] = std::shared_ptr<Targetable>(&this->libraries[players[i].id]);
		this->graveyards[players[i].id] = Zone<Card, Token>(GRAVEYARD);
		this->gameObjects[this->graveyards[players[i].id].id] = std::shared_ptr<Targetable>(&this->graveyards[players[i].id]);

		// CodeReview: Handle Land play incrementing/decrementing
		// CodeReview: Handle land plays with two counts for available/played
		this->landPlays[players[i].id] = 1;
		this->lifeTotals[players[i].id] = 20;
		this->manaPools[players[i].id] = Mana();
        for(const Card& card : libraries[i]) {
			std::shared_ptr<Targetable> copy(new Card(card));
			copy->owner = players[i].id;
			copy->id = xg::newGuid();
			this->libraries[players[i].id].addObject(copy, copy->id);
            this->gameObjects[copy->id] = copy;
        }
        
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(this->libraries[players[i].id].objects.begin(), this->libraries[players[i].id].objects.end(), g);
	}
	// Create StateQueryHandler for +1/+1 and -1/-1 counters
}



int Environment::getPower(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getPower(card);
}

int Environment::getPower(std::shared_ptr<CardToken> target) const{
	PowerQuery query{ *target, target->basePower };
	return std::get<PowerQuery>(executeStateQuery(query)).currentValue;
}

int Environment::getToughness(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getToughness(card);
}

int Environment::getToughness(std::shared_ptr<CardToken> target) const {
	ToughnessQuery query{ *target, target->baseToughness };
	return std::get<ToughnessQuery>(executeStateQuery(query)).currentValue;
}

bool Environment::goodTiming(xg::Guid target) const {
	std::shared_ptr<CostedEffect> effect = std::dynamic_pointer_cast<CostedEffect>(gameObjects.at(target));
	return this->goodTiming(effect);
}

bool Environment::goodTiming(std::shared_ptr<CostedEffect> target) const {
	bool value = false;
	if (std::shared_ptr<Card> card = std::dynamic_pointer_cast<Card>(target)) {
		std::set<CardType> types = this->getTypes(std::dynamic_pointer_cast<CardToken>(card));
		if (types.find(INSTANT) != types.end()) value = true;
		else value = (this->currentPhase == PRECOMBATMAIN || this->currentPhase == POSTCOMBATMAIN)
			&& this->stack.objects.empty()
			&& this->players[this->turnPlayer].id == card->owner;
	}
	// CodeReview: Handle abilities that are sorcery speed only
	else value = true;
	TimingQuery query{ *target, value };
	return std::get<TimingQuery>(executeStateQuery(query)).timing;
}

std::set<CardSuperType> Environment::getSuperTypes(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getSuperTypes(card);
}

std::set<CardSuperType> Environment::getSuperTypes(std::shared_ptr<CardToken> target)  const {
	SuperTypesQuery query{ *target, target->baseSuperTypes };
	return std::get<SuperTypesQuery>(executeStateQuery(query)).superTypes;
}

std::set<CardType> Environment::getTypes(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getTypes(card);
}

std::set<CardType> Environment::getTypes(std::shared_ptr<CardToken> target) const {
	TypesQuery query{ *target, target->baseTypes };
	return std::get<TypesQuery>(executeStateQuery(query)).types;
}

std::set<CardSubType> Environment::getSubTypes(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	SubTypesQuery query{ *card, card->baseSubTypes };
	return std::get<SubTypesQuery>(executeStateQuery(query)).subTypes;
}

std::set<Color> Environment::getColors(xg::Guid target)  const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	ColorsQuery query{ *card, card->baseColors };
	return std::get<ColorsQuery>(executeStateQuery(query)).colors;
}

xg::Guid Environment::getController(xg::Guid target) const {
	std::shared_ptr<Targetable> card = gameObjects.at(target);
	ControllerQuery query{ *card, card->owner };
	return std::get<ControllerQuery>(executeStateQuery(query)).controller;
}

std::vector<std::shared_ptr<ActivatedAbility>> Environment::getActivatedAbilities(xg::Guid target) const {
	std::shared_ptr<CardToken> card = std::dynamic_pointer_cast<CardToken>(gameObjects.at(target));
	return this->getActivatedAbilities(card);
}

std::vector<std::shared_ptr<ActivatedAbility>> Environment::getActivatedAbilities(std::shared_ptr<CardToken> target) const {
	ActivatedAbilitiesQuery query{ *target, target->activatableAbilities };
	return std::get<ActivatedAbilitiesQuery>(executeStateQuery(query)).abilities;
}

StateQuery& Environment::executeStateQuery(StateQuery&& query) const {
	for (std::shared_ptr<StateQueryHandler> sqh : this->stateQueryHandlers) {
		sqh->handleEvent(query, *this);
	}
	return query;
}
