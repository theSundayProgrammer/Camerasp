
INCLUDES = -I ../include
CXX_FLAGS = -std=c++11  -pthread -DASIO_STANDALONE
BUILD_DIR=./build
CXX=g++-4.9
SUBDIRS = http

all : http server
.PHONY : all

http:
	$(MAKE) -C $@

srcs = $(wildcard *.cpp)
objs = $(srcs:%.cpp=$(BUILD_DIR)/%.o)
deps = $(srcs:.cpp=$(BUILD_DIR)/.d)

server: $(objs)
	$(CXX)   -o $@ $^ -pthread  -ljpeg http/libvia-http.a -lraspicam

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $(OPTIONS) -MMD -MP -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) server

-include $(deps)
