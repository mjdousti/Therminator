#
# Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
# and Massoud Pedram. All rights reserved.
#
# Please refer to the LICENSE file for terms of use.
#

ifeq ($(shell uname -s),Darwin)
	OS_TYPE = Mac
	# Use Homebrew utilities
	export PATH : =/usr/local/bin:$(PATH)
	# Replace with the latest GNU C++ compiler
	CXX = $(shell ls /usr/local/bin/g++-* | head -n 1)
	CXXFLAGS =
	EIGEN_INC := $(shell echo /usr/local/Cellar/eigen/*/include/eigen3 | head -n 1)
	MKL_INC = $(MKLROOT)/include
	BOOST_INC := $(shell echo /usr/local/Cellar/boost/*/include | head -n 1)
	INCLUDE = -I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -I$(BOOST_INC)
	LIBRARIES =  -L$(MKLROOT)/lib -L$(MKLROOT)/../../compiler/latest/mac/compiler/lib -Wl,-rpath,${MKLROOT}/lib -lmkl_intel_ilp64 -lmkl_intel_thread -lmkl_core
else
	OS_TYPE = Linux
	CXX = g++
	LD = ld.gold
	EIGEN_INC = /usr/include/eigen3
	MKL_INC = /usr/include/mkl
	LIBRARIES = -Wl,--start-group -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -Wl,--end-group -lrt
	# NVIDIA GPUs are most probably not available for Mac machines.
	CUDA_LIB = /usr/local/cuda/lib64
	CXXFLAGS = -fopenmp
endif

TARGET = therminator2
.DEFAULT_GOAL := $(TARGET)

VPATH = src

#Default is to use CPU
ifndef
    USE_GPU = 0
endif

ifeq ($(USE_GPU), 1)
    INCLUDE = -I $(EIGEN_INC) -I /usr/local/cuda/include -I src/headers/cuda_helper 
    CXXFLAGS =
    LIBRARIES = -L $(CUDA_LIB) \
                -lcusolver \
                -lcudart \
                -lcusparse
    solver.o: solver.cpp
        /usr/local/cuda/bin/nvcc -O3 -D USE_GPU=$(USE_GPU) $(INCLUDE) -c -o $@ $< 
else
    INCLUDE += -I $(EIGEN_INC) -I $(MKL_INC)
    LIBRARIES += -ldl -lpthread -lm -liomp5
endif

CXXFLAGS += -m64 -MD -Ofast -std=c++17 $(INCLUDE) -Wall -D USE_GPU=$(USE_GPU) -march=native -fdata-sections -ffunction-sections -DNDEBUG
LDFLAGS += -Ofast $(LIBRARIES) -march=native -Wall
 

OBJ = therminator.o solver.o component.o device.o entity.o floorplan.o material.o model.o subcomponent.o parser.o physical_entity.o rc_utils.o utils.o neighbor_components.o pugixml.o

-include $(OBJ:.o=.d)

pugixml.o: src/libs/pugixml/pugixml.cpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)
# To mitigate MacOS @rpath problem (https://community.intel.com/t5/Intel-C-Compiler/dyld-Library-not-loaded-rpath-libimp5-dylib/td-p/1097284)
ifeq ($(OS_TYPE),Mac)
	install_name_tool -change @rpath/libiomp5.dylib /opt/intel/oneapi/compiler/2021.1.1/mac/compiler//lib/libiomp5.dylib $(TARGET)
endif
	strip $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJ) $(OBJ:.o=.d)

