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


bool rdma_communicate_write_imm(RdmaResourcePair& rdma, std::string remote_name, int port, std::string role)
{
	if (role == "reader")
		rdma.post_receive(rdma.get_buf(), 1);


	if (role == "writer")
	{
		rdma.rdma_write_imm(rdma.get_buf(), rdma.get_buf_size(), 0);
		return rdma.poll_completion();
	}
	else if (role == "reader")
	{
		return rdma.poll_completion();
		// msg was stored in rdma.get_buf() actually.
	}
	
}
// enum class BenchMethod: char*
// {
//     IMM_Write: "IMM Write"

// }

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
                 "             average: " << std::setprecision(3) << dura.count()*1000000 /times <<  std::endl;
}

void benchmark_write_imm(std::string remote_name, int port, std::string role, char* mem, size_t size, uint times)
{
        if (role == "reader" || role == "writer"){
        std::cerr << "[Benchmark]("<< role << ") Write IMM " <<
            "0(size="<< size<<"): " << times << "times"<< std::endl;
    } else {
        assert(false);
    }// setup rdma connection
	RdmaResourcePair rdma = rdma_setup(remote_name, port, mem, size);

    rdma.barrier(remote_name, port);

	auto t1 = steady_clock::now();
	for (int i = 0; i < times; ++ i)
	{
		rdma_communicate_write_imm(rdma, remote_name, port, role);
	}
    auto t2 = steady_clock::now();
    rdma.barrier(remote_name, port);

    auto dura = duration_cast<duration<double>>(t2-t1);
    std::cerr << "[Benchmark] Round time: " << dura.count()*1000000 << " µseconds."  <<
                 "             average: " << std::setprecision(3) << dura.count()*1000000 /times <<  std::endl;
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

    benchmark_send(remote_name, port, role, mem_rdma, size, times);
	
	return 0;
}
