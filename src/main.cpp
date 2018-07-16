#include "cards/cardManager.h"
#include "runner.h"
#include "strategy.h"

int main(int argc, char* argv[]){
    // Quiet gcc warnings about unused variables
    char* name = argv[0];
    std::vector<std::vector<std::shared_ptr<Card>>> libraries = std::vector<std::vector<std::shared_ptr<Card>>>();
    CardManager cardManager;
    libraries.push_back(std::vector<std::shared_ptr<Card>>());
    libraries.push_back(std::vector<std::shared_ptr<Card>>());
    for(int i=0; i < 40; i++) libraries[0].push_back(cardManager.getCard("Divination"));
    for(int i=0; i < 20; i++) libraries[0].push_back(cardManager.getCard("Island"));
    for(int i=0; i < 40; i++) libraries[1].push_back(cardManager.getCard("Divination"));
    for(int i=0; i < 20; i++) libraries[1].push_back(cardManager.getCard("Island"));
    std::vector<Player> players{Player(std::shared_ptr<Strategy>(new RandomStrategy())),
                                Player(std::shared_ptr<Strategy>(new RandomStrategy()))};
    Runner runner(libraries, players);
    runner.runGame();
    return argc * 0 * name[0];
}
