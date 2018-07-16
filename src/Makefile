mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))

CXX := g++-8
INCLUDES := -I$(current_dir)
CXXFLAGS := $(INCLUDES) --std=c++17 -Wall -Wextra -Wpedantic -DGUID_LIBUUID

LIBS := -luuid
OBJECTS := guid.o changeset.o mana.o cost.o card.o enum.o ability.o strategy.o player.o environment.o cards/cardManager.o runner.o main.o
SRCS := $(subst .o,.cpp,$(OBJECTS))
OUTPUT := ../MagicEngine

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O3
release: all

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(OUTPUT) $(LIBS)

depend: .depends

.depends: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(OBJECTS) $(OUTPUT)
