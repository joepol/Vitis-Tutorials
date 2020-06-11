/*******************************************************************************
Vendor: Xilinx
Associated Filename: vadd.cpp
Purpose: VITIS vector addition

*******************************************************************************
Copyright (C) 2019 XILINX, Inc.

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law:
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising under
or in connection with these materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage (including loss of data,
profits, goodwill, or any type of loss or damage suffered as a result of any
action brought by a third party) even if such damage or loss was reasonably
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES.

*******************************************************************************/
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "vadd.h"

static const int DATA_SIZE = 1024*1024;

static const std::string error_message =
    "Error: Result mismatch:\n"
    "i = %d CPU result = %d Device result = %d\n";

int main(int argc, char* argv[]) {

    //TARGET_DEVICE macro needs to be passed from gcc command line
    if(argc != 2) {
		std::cout << "Usage: " << argv[0] <<" <xclbin>" << std::endl;
		return EXIT_FAILURE;
	}

    char* xclbinFilename = argv[1];
    
    // Compute the size of array in bytes
    size_t size_in_bytes = DATA_SIZE * sizeof(short);
    
    // Creates a vector of DATA_SIZE elements with an initial value of 10 and 32
    // using customized allocator for getting buffer alignment to 4k boundary
    
    std::vector<cl::Device> devices;
    cl::Device device;
    std::vector<cl::Platform> platforms;
    bool found_device = false;

    //traversing all Platforms To find Xilinx Platform and targeted
    //Device in Xilinx Platform
    cl::Platform::get(&platforms);
    for(size_t i = 0; (i < platforms.size() ) & (found_device == false) ;i++){
        cl::Platform platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
        if ( platformName == "Xilinx"){
            devices.clear();
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
	    if (devices.size()){
		    device = devices[0];
		    found_device = true;
		    break;
	    }
        }
    }
    if (found_device == false){
       std::cout << "Error: Unable to find Target Device " 
           << device.getInfo<CL_DEVICE_NAME>() << std::endl;
       return EXIT_FAILURE; 
    }

    // Creating Context and Command Queue for selected device
    cl::Context context(device);
    cl::CommandQueue q(context, device, cl::QueueProperties::Profiling | cl::QueueProperties::OutOfOrder);


    // Load xclbin 
    std::cout << "Loading: '" << xclbinFilename << "'\n";
    std::ifstream bin_file(xclbinFilename, std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    unsigned nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    
    // Creating Program from Binary File
    cl::Program::Binaries bins;
    bins.push_back({buf,nb});
    devices.resize(1);
    cl::Program program(context, devices, bins);
    
    // This call will get the kernel object from program. A kernel is an 
    // OpenCL function that is executed on the FPGA. 
    cl::Kernel krnl_vector_add(program,"krnl_vadd");
    

	// These commands will allocate memory on the Device. The cl::Buffer objects can
	// be used to reference the memory locations on the device.

    //first kernel
	cl::Buffer buffer_in1_re(context, CL_MEM_READ_ONLY, size_in_bytes);
	cl::Buffer buffer_in1_im(context, CL_MEM_READ_ONLY, size_in_bytes);
	cl::Buffer buffer_in2_re(context, CL_MEM_READ_ONLY, size_in_bytes);
	cl::Buffer buffer_in2_im(context, CL_MEM_READ_ONLY, size_in_bytes);
	cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, size_in_bytes);

	int narg=0;
	krnl_vector_add.setArg(narg++,buffer_in1_re);
	krnl_vector_add.setArg(narg++,buffer_in1_im);
	krnl_vector_add.setArg(narg++,buffer_in2_re);
	krnl_vector_add.setArg(narg++,buffer_in2_im);
	krnl_vector_add.setArg(narg++,buffer_out);
	krnl_vector_add.setArg(narg++,DATA_SIZE);
	krnl_vector_add.setArg(narg++,0);

	//We then need to map our OpenCL buffers to get the pointers
	short *ptr_in1_re = (short *) q.enqueueMapBuffer (buffer_in1_re , CL_TRUE , CL_MAP_WRITE , 0, size_in_bytes);
	short *ptr_in1_im = (short *) q.enqueueMapBuffer (buffer_in1_im , CL_TRUE , CL_MAP_WRITE , 0, size_in_bytes);
	short *ptr_in2_re = (short *) q.enqueueMapBuffer (buffer_in2_re , CL_TRUE , CL_MAP_WRITE , 0, size_in_bytes);
	short *ptr_in2_im = (short *) q.enqueueMapBuffer (buffer_in2_im , CL_TRUE , CL_MAP_WRITE , 0, size_in_bytes);
	short *ptr_res = (short *) q.enqueueMapBuffer (buffer_out , CL_TRUE , CL_MAP_READ , 0, size_in_bytes);

	//setting input data
	for(int i = 0 ; i < DATA_SIZE; i++){
		ptr_in1_re[i] = 1;
		ptr_in1_im[i] = 2;
		ptr_in2_re[i] = 3;
		ptr_in2_im[i] = 4;
	}

	for (int var1 = 0; var1 < 50; ++var1) {
        // Data will be migrated to kernel space
        auto start_all = std::chrono::high_resolution_clock::now();

		cl::Event write_event;
        q.enqueueMigrateMemObjects({buffer_in1_re,buffer_in1_im,buffer_in2_re,buffer_in2_im},0, nullptr, &write_event/* 0 means from host*/);

        std::vector<cl::Event> iteration_events{write_event};

        std::vector<cl::Event> task_events;

        cl::Event task_event;

		q.enqueueTask(krnl_vector_add, &iteration_events, &task_event);

		task_events.push_back(task_event);

		iteration_events.insert(iteration_events.end(), task_events.begin(), task_events.end());

		cl::Event read_event;

        //time measurement for migrating data from DDR to host memory
        auto start = std::chrono::high_resolution_clock::now();

        q.enqueueMigrateMemObjects({buffer_out},CL_MIGRATE_MEM_OBJECT_HOST, &iteration_events, &read_event);

        iteration_events.push_back(read_event);

        q.finish();

        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        auto elapsed_all = std::chrono::high_resolution_clock::now() - start_all;
        auto microseconds_all = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_all).count();
        printf("time for write back = %d [us]\n",microseconds);
        printf("time for all = %d [us]\n",microseconds_all);


        //Verify the result
       int match = 0;
	   for (int i = 0; i < DATA_SIZE; i++) {
		   int host_result_real = ptr_in1_re[i] * ptr_in2_re[i] - (ptr_in1_im[i] * ptr_in2_im[i]);
           int host_result_img = ptr_in1_re[i] * ptr_in2_im[i] + (ptr_in1_im[i] * ptr_in2_re[i]);
           int host_result_power = host_result_real * host_result_real + host_result_img * host_result_img;

           if (host_result_power != ptr_res[i]  ) {
                printf(error_message.c_str(), i, host_result_power, ptr_res[i]);
                match = 1;

           }
       }
     }

        q.enqueueUnmapMemObject(buffer_in1_re , ptr_in1_re);
        q.enqueueUnmapMemObject(buffer_in1_im , ptr_in1_im);
        q.enqueueUnmapMemObject(buffer_in2_re , ptr_in2_re);
        q.enqueueUnmapMemObject(buffer_in2_im , ptr_in2_im);
        q.enqueueUnmapMemObject(buffer_out , ptr_res);

        q.finish();





    return 0;

}
