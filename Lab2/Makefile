_SOURCES	   := $(wildcard src/*.cpp)
_OBJECTS	   := $(patsubst %.cpp,%.o,$(_SOURCES))
_DEPS		   := $(wildcard src/*.h)

CLIENT_SOURCES := $(wildcard src/client/*.cpp) $(_SOURCES)
CLIENT_OBJECTS := $(patsubst %.cpp,%.o,$(CLIENT_SOURCES))
CLIENT_DEPS    := $(wildcard src/client/*.h) $(_DEPS)

SERVER_SOURCES := $(wildcard src/server/*.cpp) $(_SOURCES)
SERVER_OBJECTS := $(patsubst %.cpp,%.o,$(SERVER_SOURCES))
SERVER_DEPS    := $(wildcard src/server/*.h) $(_DEPS)

IDIR = src/
CPPFLAGS=-g -std=gnu++0x -I$(IDIR)

all: client server

client: $(CLIENT_OBJECTS) $(CLIENT_DEPS)
	$(CXX) $(CLIENT_OBJECTS) -o bin/client

server: $(SERVER_OBJECTS) $(SERVER_DEPS)
	$(CXX) $(SERVER_OBJECTS) -o bin/server

src/client/%.o: src/client/%.cpp $(CLIENT_DEPS)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

src/server/%.o: src/server/%.cpp $(SERVER_DEPS)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

src/%.o: src/%.cpp $(_DEPS)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

clean:
	rm $(CLIENT_OBJECTS) $(SERVER_OBJECTS)
