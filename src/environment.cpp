#include <algorithm>
#include <random>

#include "environment.h"

Environment::Environment(std::vector<Player>& prelimPlayers, std::vector<std::vector<Card>>& libraries)
: players(prelimPlayers), graveyard(GRAVEYARD), battlefield(BATTLEFIELD), stack(STACK), exile(EXILE), command(COMMAND),
  currentPhase(UNTAP), currentPlayer(0), turnPlayer(0)
{
	this->gameObjects[graveyard.id] = std::shared_ptr<Targetable>(&graveyard);
	this->gameObjects[battlefield.id] = std::shared_ptr<Targetable>(&battlefield);
	this->gameObjects[stack.id] = std::shared_ptr<Targetable>(&stack);
	this->gameObjects[exile.id] = std::shared_ptr<Targetable>(&exile);
	this->gameObjects[command.id] = std::shared_ptr<Targetable>(&command);
    for(unsigned int i=0; i < players.size(); i++) {
        this->gameObjects[players[i].id] = std::shared_ptr<Targetable>(&players[0]);
        this->hands[players[i].id] = Zone<Card, Token>(HAND);
		this->gameObjects[this->hands[players[i].id].id] = std::shared_ptr<Targetable>(&this->hands[players[i].id]);
        this->libraries[players[i].id] = Zone<Card, Token>(LIBRARY);
		this->landPlays[players[i].id] = 1;

		this->gameObjects[this->libraries[players[i].id].id] = std::shared_ptr<Targetable>(&this->libraries[players[i].id]);
        this->lifeTotals[players[i].id] = 20;
		this->manaPools[players[i].id] = Mana();
        for(Card card : libraries[i]) {
			Card* copy = new Card(card);
			copy->owner = players[i].id;
			copy->id = xg::newGuid();
            this->gameObjects[copy->id] = std::shared_ptr<Card>(copy);
			std::shared_ptr<Targetable> ptr(copy);
            this->libraries[players[i].id].addObject(ptr, copy->id);
        }
        
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(this->libraries[players[i].id].objects.begin(), this->libraries[players[i].id].objects.end(), g);
	}
}
