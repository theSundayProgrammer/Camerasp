INCLUDES = -I ~/boostlib/include
CXX_FLAGS = -std=c++11 -pthread -DASIO_STANDALONE
BUILD_DIR=./build
CXX=g++-4.9


srcs = $(wildcard *.cpp)
objs = $(srcs:%.cpp=$(BUILD_DIR)/%.o)

libvia-http.a: $(objs)
	ar rcs libvia-http.a $^

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(INCLUDES)   -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) 

