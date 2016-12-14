
INCLUDES = -I ../include -I . 
CXX_FLAGS = -pthread 
BUILD_DIR=./build
CXX=g++-6

all : camerasp
.PHONY : all

http:
	$(MAKE) -C $@


camerasp: $(BUILD_DIR)/raspicam_still_test.o $(BUILD_DIR)/cam_still.o    
	$(CXX)   -o $@ $^ -pthread -L/opt/vc/lib -lmmal -lmmal_core -lmmal_util 

$(BUILD_DIR)/cam_still.o: ../src/cam_still.cpp
	$(CXX) $(CXX_FLAGS) $(INCLUDES)   -c $< -o $@

$(BUILD_DIR)/raspicam_still_test.o: raspicam_still_test.cpp
	$(CXX) $(CXX_FLAGS) $(INCLUDES)   -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(BUILD_DIR)/*.o  camerasp


