CXX := g++-8
CXXFLAGS := --std=c++17 -Wall -Wextra -Wpedantic
INCLUDES := -I.

LIBS := -L../libs -lcrossguid
OBJECTS := mana.o cost.o card.o ability.o strategy.o player.o environment.o runner.o main.o
OUTPUT := ../MagicEngine

my_program: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(OUTPUT) $(LIBS)

%.o: %.cxx
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(inputs) -o $(output)

clean:
	rm *.o $(OUTPUT)
