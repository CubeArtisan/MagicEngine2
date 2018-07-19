#include "cards/cardManager.h"
#include "runner.h"
#include "strategy.h"

int main(){
	std::vector<std::vector<Card>> libraries{ std::vector<Card>(), std::vector<Card>() };
    CardManager cardManager;
	libraries[0].reserve(60);
	libraries[1].reserve(60);
    for(int i=0; i < 40; i++) libraries[0].push_back(cardManager.getCard("Bronze Sable"));
    for(int i=0; i < 20; i++) libraries[0].push_back(cardManager.getCard("Island"));
    for(int i=0; i < 20; i++) libraries[1].push_back(cardManager.getCard("Goblin Instigator"));
	for(int i=0; i < 20; i++) libraries[1].push_back(cardManager.getCard("Impact Tremors"));
    for(int i=0; i < 20; i++) libraries[1].push_back(cardManager.getCard("Mountain"));
    std::vector<Player> players{Player(std::shared_ptr<Strategy>(new RandomStrategy())),
                                Player(std::shared_ptr<Strategy>(new RandomStrategy()))};
    Runner runner(libraries, players);
	for (int i = 0; i < 1; i++) {
		runner.runGame();
		std::cout << i << std::endl;
	}
}
