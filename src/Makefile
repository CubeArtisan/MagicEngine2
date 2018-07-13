CXX := g++-8
CXXFLAGS := --std=c++17 -Wall -Wextra -Wpedantic -DGUID_LIBUUID
INCLUDES := -I.

LIBS := 
OBJECTS := guid.o changeset.o mana.o cost.o card.o ability.o strategy.o player.o environment.o runner.o main.o
OUTPUT := ../MagicEngine

$(OUTPUT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(OUTPUT) $(LIBS)

%.o: %.cxx
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(inputs) -o $(output)

clean:
	rm *.o $(OUTPUT)
