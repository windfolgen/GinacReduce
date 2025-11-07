# compiler
CXX := g++
CXXFLAGS := -fPIC -shared -O3 -std=c++17

INCLUDES := -I/Applications/Mathematica.app/Contents/SystemFiles/IncludeFiles/C -I/opt/homebrew/Cellar/ginac/1.8.8/include -I/usr/local/include

LDFLAGS := -L/opt/homebrew/Cellar/ginac/1.8.8/lib -L/usr/local/lib

# target
all: ginacreduce

# executable file
ginacreduce: ginac.cpp
	$(CXX) $(CXXFLAGS) ginac.cpp -o ginacreduce.dylib $(INCLUDES) $(LDFLAGS) -lginac -lcln

# clean target
clean:
	rm -f ginacreduce.dylib