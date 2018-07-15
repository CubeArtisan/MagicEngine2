#include "environment.h"

Environment::Environment(std::vector<Player>& players, std::vector<std::vector<std::shared_ptr<Card>>>& libraries)
: players(players), currentPlayer(0), turnPlayer(0)
{
    for(unsigned int i=0; i < players.size(); i++) {
        this->hands[players[i].id] = Zone<Card, Token>();
        this->libraries[players[i].id] = Zone<Card, Token>();
        this->lifeTotals[players[i].id] = 20;
        for(std::shared_ptr<Card> card : libraries[i]) {
            this->gameObjects[card->id] = card;
            this->libraries[players[i].id].addObject(*card, card->id);
        }
    }
}
