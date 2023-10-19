SOURCES := $(wildcard *.cpp)
OBJECTSDIR = obj

#obj/<object>.o
OBJECTS := $(addprefix $(OBJECTSDIR)/,$(SOURCES:.cpp=.o))

CXX = g++-12

CPPFLAGS = -std=c++20 -Wall -g

all: createFolder $(OBJECTS) main

main: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(OBJECTS) -o $@

createFolder:
	mkdir -p $(OBJECTSDIR)

$(OBJECTS) : $(OBJECTSDIR)/%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $< -o $@