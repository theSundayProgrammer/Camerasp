
INCLUDES = -I ../include
CXX_FLAGS = -pthread -DASIO_STANDALONE
BUILD_DIR=./build
CXX=g++-6
SUBDIRS = http

all : http camerasp
.PHONY : all

http:
	$(MAKE) -C $@

srcs = $(wildcard *.cpp)
objs = $(srcs:%.cpp=$(BUILD_DIR)/%.o)
deps = $(srcs:.cpp=$(BUILD_DIR)/.d)

camerasp: $(objs)
	$(CXX)   -o $@ $^ -pthread -L/opt/vc/lib -ljpeg http/libvia-http.a -lmmal -lmmal_core -lmmal_util

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $(OPTIONS) -MMD -MP -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) camerasp

-include $(deps)
