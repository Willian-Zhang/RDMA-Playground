
CXX=g++
CPPFLAGS=-std=c++14 #-w
HEADER=#-I/data/miao/lib/libibverbs-1.2.0/include
LIBB=#-L/data/miao/lib/libibverbs-1.2.0/lib
LDFLAGS=-lzmq -libverbs
#ibv_devinfo -v

all: rdma_demo_1side rdma_demo_2sides rdma_benchmark

rdma_resouce.o: rdma_resource.hpp rdma_resource.cpp
	$(CXX) $(CPPFLAGS) $(HEADER) $(LIBB) $(LDFLAGS) rdma_resource.cpp -c

rdma_demo_1side: rdma_demo_1side.cpp rdma_resource.o
	$(CXX) $(CPPFLAGS) $(HEADER) rdma_demo_1side.cpp -o rdma_demo_1side rdma_resource.o $(LIBB) $(LDFLAGS) 

rdma_demo_2sides: rdma_demo_2sides.cpp rdma_resource.o
	$(CXX) $(CPPFLAGS) $(HEADER) rdma_demo_2sides.cpp -o rdma_demo_2sides rdma_resource.o $(LIBB) $(LDFLAGS) 

rdma_benchmark: rdma_benchmark.cpp rdma_resource.o
	$(CXX) $(CPPFLAGS) $(HEADER) rdma_benchmark.cpp -o rdma_benchmark rdma_resource.o $(LIBB) $(LDFLAGS) 


clean:
	-rm rdma_demo_1side rdma_demo_2sides rdma_benchmark
