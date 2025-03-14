CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3
TARGET= arp_client
OBJS = main.o interface_manager.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp interface_manager.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

interface_manager.o: interface_manager.cpp interface_manager.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
