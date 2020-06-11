
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "host.hpp"


static const int DATA_SIZE = 1024*1024;

static const std::string error_message =
    "Error: Result mismatch:\n"
    "i = %d CPU result = %d Device result = %d\n";

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
		return EXIT_FAILURE;
	}

	std::string binaryFile = argv[1];
	size_t vector_size_bytes = sizeof(int) * DATA_SIZE;
	cl_int err;
	unsigned fileBufSize;

	std::vector < cl::Device > devices = get_devices("Xilinx");
	devices.resize(1);
	cl::Device device = devices[0];

	char* fileBuf = read_binary_file(binaryFile, fileBufSize);
	cl::Program::Binaries bins { { fileBuf, fileBufSize } };



    // Creating Context and Command Queue for selected device
    cl::Context context(device);
    cl::CommandQueue q(context, device, cl::QueueProperties::Profiling | cl::QueueProperties::OutOfOrder);


    cl::Program program(context, devices, bins);

    // This call will get the kernel object from program. A kernel is an
    // OpenCL function that is executed on the FPGA.
    cl::Kernel minKernel(program,"min_kernel");


	// These commands will allocate memory on the Device. The cl::Buffer objects can
	// be used to reference the memory locations on the device.

    //first kernel
    size_t size_in_bytes = DATA_SIZE * sizeof(int);
	cl::Buffer buffer_in(context, CL_MEM_READ_ONLY, size_in_bytes);
	cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, sizeof(int));
//	int result = 0;

	int narg=0;
	minKernel.setArg(narg++, buffer_in);
	minKernel.setArg(narg++, buffer_out);
	minKernel.setArg(narg++, DATA_SIZE);


	//We then need to map our OpenCL buffers to get the pointers
	int *ptr_in = (int *) q.enqueueMapBuffer (buffer_in , CL_TRUE , CL_MAP_WRITE , 0, size_in_bytes);
	int *ptr_res = (int *) q.enqueueMapBuffer (buffer_out , CL_TRUE , CL_MAP_READ , 0, sizeof(int));

	//setting input data
	ptr_in[0] = 50;
	ptr_in[1] = 50;
	ptr_in[2] = 50;
	ptr_in[3] = 50;
	ptr_in[4] = 50;
	int counter = 20;
	for(int i = 5 ; i < DATA_SIZE; i++){
		ptr_in[i] = counter++;
	}
	ptr_in[524] = 2;

//	for (int var1 = 0; var1 < 50; ++var1) {
//        // Data will be migrated to kernel space
//        auto start_all = std::chrono::high_resolution_clock::now();
//
		cl::Event write_event;
        q.enqueueMigrateMemObjects({buffer_in},0, nullptr, &write_event/* 0 means from host*/);
//
        std::vector<cl::Event> iteration_events{write_event};
//
        std::vector<cl::Event> task_events;
//
        cl::Event task_event;
//
		q.enqueueTask(minKernel, &iteration_events, &task_event);
//
		task_events.push_back(task_event);
//
		iteration_events.insert(iteration_events.end(), task_events.begin(), task_events.end());
//
		cl::Event read_event;
//
//        //time measurement for migrating data from DDR to host memory
        auto start = std::chrono::high_resolution_clock::now();
//
        q.enqueueMigrateMemObjects({buffer_out},CL_MIGRATE_MEM_OBJECT_HOST, &iteration_events, &read_event);
//
        iteration_events.push_back(read_event);
//
        q.finish();
//
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

//        auto elapsed_all = std::chrono::high_resolution_clock::now() - start_all;
//        auto microseconds_all = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_all).count();
//        printf("time for write back = %d [us]\n",microseconds);
//        printf("time for all = %d [us]\n",microseconds_all);

        std::cout << "Result, index of minimal value is : " << ptr_res[0] << std::endl;
        std::cout << "Time back and forth with computation" << microseconds << std::endl;
//
//	}

        q.enqueueUnmapMemObject(buffer_in , ptr_in);
        q.finish();


    return 0;

}
