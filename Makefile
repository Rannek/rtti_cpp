CXX = g++
CXXFLAGS = -std=c++17 -I.
TARGET = thumbnail_extractor
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)