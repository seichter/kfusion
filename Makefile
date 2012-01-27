CUDA_ROOT=/usr/local/cuda

CXX = $(CUDA_ROOT)/bin/nvcc
CC = $(CUDA_ROOT)/bin/nvcc
CPPFLAGS=-I../include -I$(CUDA_ROOT)/include -I/opt/local/include
CXXFLAGS=-g -m64 -O3 -use_fast_math
LDFLAGS=-g -m64 -L../lib -lfreenect -Xlinker -framework,OpenGL,-framework,GLUT

%.o: %.cu
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^

all: kinect test

test: kfusion.o helpers.o test.o

kinect: kfusion.o helpers.o kinect.o

clean:
	rm *.o test kinect

