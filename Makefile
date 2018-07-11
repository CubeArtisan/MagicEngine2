CXX := g++-8
CXXFLAGS := --std=c++17
INCLUDES := -I.

LIBS := -L. -lcrossguid
OBJECTS := mana.o environment.o main.o

my_program: $(OBJECTS)
	$(CXX) $(OBJECTS) -o MagicEngine $(LIBS)

%.o: %.cxx
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(inputs) -o $(output)

clean:
	rm *.o MagicEngine
