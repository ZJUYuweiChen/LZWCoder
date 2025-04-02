CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -lboost_filesystem -lboost_system

TARGET = lzwcoder
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: CXXFLAGS += -w -O2
all: $(TARGET)
	rm -f $(OBJS)

debug: CXXFLAGS += -Wall -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all debug run clean