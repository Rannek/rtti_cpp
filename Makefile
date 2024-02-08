CXX = g++
CXXFLAGS = -std=c++17 -I. `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`
TARGET = thumbnail_extractor
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
