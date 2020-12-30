#include "rdma_resource.hpp"
#include <iostream>
#include <iomanip>

using namespace std::chrono;


RdmaResourcePair rdma_setup(std::string remote_name, int port, char* mem, size_t size)
{
	RdmaResourcePair rdma(mem, size);
	rdma.exchange_info(remote_name, port);
	return rdma;
}

bool rdma_communicate_send(RdmaResourcePair& rdma, std::string remote_name, int port, std::string role, bool last)
{
	
	if (role == "reader")
	{
		rdma.barrier(remote_name, port);
	
		// Dummy headed
		// rdma.post_receive(rdma.get_buf(), rdma.get_buf_size());	
		// ...
		// std::cerr << "...";
		rdma.poll_completion();
		// std::cerr << "[ ]";
		rdma.send(rdma.get_buf(), rdma.get_buf_size(), 0);
		// TODO: Retry
		// std::cerr << "<-";
		rdma.poll_completion();
		//  next-head:
		if(!last) {
			// std::cerr << "O";
			rdma.post_receive(rdma.get_buf(), rdma.get_buf_size());	
		}
		rdma.barrier(remote_name, port);
	}

	if (role == "writer")
	{
		rdma.send(rdma.get_buf(), rdma.get_buf_size(), 0);
		// std::cerr << "-";
		rdma.poll_completion();
		// std::cerr << ">";	
		rdma.post_receive(rdma.get_buf(), rdma.get_buf_size());	
		// std::cerr << "...";
		rdma.barrier(remote_name, port);

		// ...
		rdma.barrier(remote_name, port);
		
		auto r = rdma.poll_completion();
		// std::cerr << "[ ]";	
		return r;
	}
	// std::cerr << std::endl;
	return 0;
}

void benchmark_send(std::string remote_name, int port, std::string role, char* mem, size_t size, uint times)
{
    if (role == "reader" || role == "writer"){
        std::cerr << "[Benchmark]("<< role << ") Send " <<
            "0(size="<< size<<"): " << times << " times"<< std::endl;
    } else {
        assert(false);
    }
	// setup rdma connection
	RdmaResourcePair rdma = rdma_setup(remote_name, port, mem, size);

	if (role == "reader")
	{	
		// Dummy headed
		rdma.post_receive(rdma.get_buf(), rdma.get_buf_size());	
	}

	rdma.barrier(remote_name, port);

	auto t1 = steady_clock::now();
	for (int i = times; i > 0; -- i)
	{
		rdma_communicate_send(rdma, remote_name, port, role, i<=1);
	}
    auto t2 = steady_clock::now();
    rdma.barrier(remote_name, port);

    auto dura = duration_cast<duration<double>>(t2-t1);
    std::cerr << "[Benchmark] Round time: " << dura.count()*1000000 << " µseconds."  <<
                 "             average: " << std::setprecision(3)  << 
				 						dura.count()*1000000 /times << " µseconds." <<  std::endl;
}


bool rdma_communicate_write(RdmaResourcePair& rdma, std::string remote_name, int port, std::string role, char* msg, bool last)
{	
	auto offset_buf = rdma.get_buf()+1;
	if (role == "writer")
	{
		rdma.rdma_write(offset_buf, strlen(offset_buf), 0);
		rdma.poll_completion();
		char* res;
		rdma.busy_read(&res);
		// ...
		rdma.reset_buffer();
	}
	else if (role == "reader")
	{
		char* res;
		rdma.busy_read(&res);
		// ...
		rdma.reset_buffer();
		rdma.rdma_write(offset_buf, strlen(offset_buf), 0);
		rdma.poll_completion();	
	}
	
}
// enum class BenchMethod: char*
// {
//     IMM_Write: "IMM Write"

// }



void benchmark_write(std::string remote_name, int port, std::string role, char* mem, size_t size, uint times)
{
    if (role == "reader" || role == "writer"){
        std::cerr << "[Benchmark]("<< role << ") Write IMM " <<
            "0(size="<< size<<"): " << times << "times"<< std::endl;
    } else {
        assert(false);
    }// setup rdma connection
	RdmaResourcePair rdma = rdma_setup(remote_name, port, mem, size);

	// char* msg = (char*)malloc(rdma.get_buf_size());
	char* offset_msg = reinterpret_cast<char*>(rdma.get_buf()+1);
	if (role == "writer")
	{
		strcpy(offset_msg, "ping");
	}
	else if (role == "reader")
	{
		strcpy(offset_msg, "pong");
	}
	rdma.reset_buffer();

    rdma.barrier(remote_name, port);

	auto t1 = steady_clock::now();
	for (int i = times; i > 0; -- i)
	{
		rdma_communicate_write(rdma, remote_name, port, role, offset_msg, i<=1);
	}
    auto t2 = steady_clock::now();
    rdma.barrier(remote_name, port);

    auto dura = duration_cast<duration<double>>(t2-t1);
    std::cerr << "[Benchmark] Total time: " << dura.count()*1000000 << " µseconds for " 
											<< times <<" roundtrips."  << std::endl <<
                 "            average round trip: " << std::setprecision(3)  << 
				 						dura.count()*1000000 /times << " µseconds." <<  std::endl <<
				 "            average half round (single) trip: " << std::setprecision(3)  << 
				 						dura.count()*1000000 /times/2 << " µseconds." <<  std::endl <<						 
										 ;
}
int main(int argc, char** argv)
{
	assert(argc == 4);
	std::string remote_name = argv[1];
	int port = 12333;
	std::string role = argv[2];
	
    auto times = std::stoi(argv[3]);
    const auto size = 8;
    char* mem_rdma = (char*)malloc(size);

    benchmark_write(remote_name, port, role, mem_rdma, size, times);
	
	return 0;
}
