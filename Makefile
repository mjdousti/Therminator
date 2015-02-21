#Default is to use CPU
ifndef
	SOLVER = CPU
endif

ifeq ($(SOLVER), GPU)
	INCLUDE = -I/usr/local/cula/include 
	LIBRARIES =  -L/usr/local/cula/lib64 -L/usr/local/cuda/lib64  -lm -lcula_lapack -lcublas -lcudart -liomp5
	CXXFLAGS = -O3 ${INCLUDE}  -Wall -D USE_GPU=1
else
	INCLUDE =
	LIBRARIES = -lm
	CXXFLAGS = -O3 -Wall -D USE_GPU=0
endif

VPATH = src
CXX = g++
LDFLAGS = -O3 ${LIBRARIES}  -Wall
TARGET = therminator
OBJ = src/libs/pugixml/pugixml.o therminator.o component.o device.o entity.o floorplan.o material.o model.o subcomponent.o parser.o physical_entity.o rc_utils.o utils.o


$(TARGET) : $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(TARGET)
clean:
	rm -f *.o $(TARGET) $(TARGET).exe src/libs/pugixml/*.o
