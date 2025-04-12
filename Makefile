CXX := clang++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++23
SRCS := main.cpp cell.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := maze.hpp cell_metadata.hpp graph_cell.hpp cell.hpp direction.hpp
TARGET := maze

all : $(TARGET)

$(TARGET) : $(OBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
