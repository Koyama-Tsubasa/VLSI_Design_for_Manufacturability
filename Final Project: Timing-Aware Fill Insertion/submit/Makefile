EXE_TARGET = ./Fill_Insertion

CXX = g++

OFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3 -DNDEBUG

LINKFLAGS = -Ofast -march=native -std=c++1z 

INCLUDE := ./include
SRC_DIRS := ./src

SRCS := $(wildcard $(SRC_DIRS:=/*.cpp))
OBJS := $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

ifndef BOOST_LIBRARY_PATH
BOOST_LIBRARY_PATH := "/usr/local/include/boost/"
endif

all: $(EXE_TARGET)

$(EXE_TARGET): $(OBJS) 
	$(CXX) -fopenmp -o $@ $^

%.o: %.cpp
	$(CXX) -fopenmp $(LINKFLAGS) $(WARNINGFLAGS) $(OPENMPFLAG) -I $(INCLUDE) -isystem $(BOOST_LIBRARY_PATH) -MMD -c $< -o $@

clean:
	rm -rf $(EXE_TARGET) $(OBJS) $(DEPS)