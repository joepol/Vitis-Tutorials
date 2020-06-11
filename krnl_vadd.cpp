/*******************************************************************************
Vendor: Xilinx
Associated Filename: krnl_vadd.cpp
Purpose: Vitis vector addition example
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

//------------------------------------------------------------------------------
//
// kernel:  vadd
//
// Purpose: Demonstrate Vector Add Kernel
//

#include "ap_int.h"
#include <hls_stream.h>


#define BUFFER_SIZE 	8 * 2048	//required buffer size to be used in order to work on local data
#define DATAWIDTH 		128	//data width to be used in the gmem
#define DATATYPE_SIZE 	16		//real data type (short)
#define NUM_ELEMENTS 	(DATAWIDTH / DATATYPE_SIZE) // number of elements in one type of DATAWIDTH


typedef ap_uint<DATAWIDTH> uint512_dt;
typedef ap_uint<DATATYPE_SIZE> din_type;
typedef ap_uint<DATATYPE_SIZE + 1> dout_type;

//TRIPCOUNT identifier
const unsigned int c_chunk_sz = BUFFER_SIZE;
const unsigned int c_size = NUM_ELEMENTS;




void read(
		const uint512_dt *in1_re, // Read-Only Vector 1 Real Part
		const uint512_dt *in1_im, 	// Read-Only Vector 1 Imag Part
		const uint512_dt *in2_re, // Read-Only Vector 2 Real Part
		const uint512_dt *in2_im,
		hls::stream<uint512_dt> &in1_re_Stream,
		hls::stream<uint512_dt> &in1_im_Stream,
		hls::stream<uint512_dt> &in2_re_Stream,
		hls::stream<uint512_dt> &in2_im_Stream,
		int i)
{
	read1:
			for (int j = 0; j < BUFFER_SIZE; j++) {
			   #pragma HLS PIPELINE II=1
			   #pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz

				in1_re_Stream << in1_re[i + j];
				in1_im_Stream << in1_im[i + j];
				in2_re_Stream << in2_re[i + j];
				in2_im_Stream << in2_im[i + j];
			}
}




void compute(
		hls::stream<uint512_dt> &in1_re_Stream,
		hls::stream<uint512_dt> &in1_im_Stream,
		hls::stream<uint512_dt> &in2_re_Stream,
		hls::stream<uint512_dt> &in2_im_Stream,
		hls::stream<uint512_dt> &out_stream

		)
{
	for (int j = 0; j < BUFFER_SIZE; j++) {
				#pragma HLS PIPELINE II=1
				#pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz

				uint512_dt tmpV1 = in1_re_Stream.read();
				uint512_dt tmpV2 = in1_im_Stream.read();
				uint512_dt tmpV3 = in2_re_Stream.read();
				uint512_dt tmpV4 = in2_im_Stream.read();
				uint512_dt tmpOut1 = 0;
				din_type val1, val2, val3, val4;
				dout_type res_re;
				dout_type res_im;
				dout_type res_power;

				parallel_add:
				for (int i = 0; i < NUM_ELEMENTS; i++) {
					#pragma HLS UNROLL
					val1 = tmpV1.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
					val2 = tmpV2.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
					val3 = tmpV3.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
					val4 = tmpV4.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);

					res_re = ((val1 * val3) - (val2 * val4));
					res_im = ((val2 * val3) + (val1 * val4));
					res_power = res_re * res_re + res_im * res_im;
					tmpOut1.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE) = res_power;
				}
				out_stream << tmpOut1;

	}
}


void write(
		uint512_dt *out,     	// Output Result Real Part
		hls::stream<uint512_dt> &out_stream,

		int i)
{
	for (int j = 0; j < BUFFER_SIZE; j++) {
				#pragma HLS PIPELINE II=1
				#pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz

				out[i + j] = out_stream.read();
	}
}





extern "C" {

void krnl_vadd(
		const uint512_dt *in1_re, // Read-Only Vector 1 Real Part
		const uint512_dt *in1_im, 	// Read-Only Vector 1 Imag Part
		const uint512_dt *in2_re, // Read-Only Vector 2 Real Part
		const uint512_dt *in2_im, 	// Read-Only Vector 2 Imag Part
		uint512_dt *out,     	// Output Result Real Part
		int size,         			// number  of shorts (16 bits) data
		int offset)					// Offset in the array to start the calculation
{

	#pragma HLS INTERFACE m_axi port = in1_re offset = slave bundle = gmem0
	#pragma HLS INTERFACE m_axi port = in1_im offset = slave bundle = gmem1
	#pragma HLS INTERFACE m_axi port = in2_re offset = slave bundle = gmem2
	#pragma HLS INTERFACE m_axi port = in2_im offset = slave bundle = gmem3
	#pragma HLS INTERFACE m_axi port = out offset = slave bundle = gmem0
	#pragma HLS INTERFACE s_axilite port = in1_real bundle = control
	#pragma HLS INTERFACE s_axilite port = in1_img bundle = control
	#pragma HLS INTERFACE s_axilite port = in2_real bundle = control
	#pragma HLS INTERFACE s_axilite port = in2_img bundle = control
	#pragma HLS INTERFACE s_axilite port = out bundle = control
	#pragma HLS INTERFACE s_axilite port = size bundle = control
	#pragma HLS INTERFACE s_axilite port = offset bundle = control
	#pragma HLS INTERFACE s_axilite port = return bundle = control


	hls::stream<uint512_dt> in1_re_Stream("in1");
	hls::stream<uint512_dt> in1_im_Stream("in2");
	hls::stream<uint512_dt> in2_re_Stream("in3");
	hls::stream<uint512_dt> in2_im_Stream("in4");
	hls::stream<uint512_dt> out_Stream("out1");

#pragma HLS STREAM variable = in1_re_Stream depth = 1024
#pragma HLS STREAM variable = in1_im_Stream depth = 1024
#pragma HLS STREAM variable = in2_re_Stream depth = 1024
#pragma HLS STREAM variable = in2_im_Stream depth = 1024
#pragma HLS STREAM variable = out_Stream depth = 1024
	//kernel is directly accessing 512bit data elements per iteration,
	//so total number of read from global memory is calculated here:
	int size_in32 = (size - 1) / NUM_ELEMENTS + 1;

    //Per iteration of this loop perform BUFFER_SIZE vector addition
    for (int i = 0; i < size_in32; i += BUFFER_SIZE)
    {
		#pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz/c_size max=c_chunk_sz/c_size
        int chunk_size = BUFFER_SIZE;
#pragma HLS dataflow

        read(
        	in1_re,
			in1_im,
			in2_re,
			in2_im,
			in1_re_Stream,
			in1_im_Stream,
			in2_re_Stream,
			in2_im_Stream,
			i);

        compute(
        	in1_re_Stream,
			in1_im_Stream,
			in2_re_Stream,
			in2_im_Stream,
			out_Stream);

        write(
        	out,
			out_Stream,
			i);
        //burst read from global memory to local memory

//        read1:
//		for (int j = 0; j < chunk_size; j++) {
//		   #pragma HLS PIPELINE II=1
//		   #pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
//
//			in1_re_buffer[j] = in1_re[i + j];
//			in1_im_buffer[j] = in1_im[i + j];
//			in2_re_buffer[j] = in2_re[i + j];
//			in2_im_buffer[j] = in2_im[i + j];
//		}
//
//		//compute
//		compute:
//		for (int j = 0; j < BUFFER_SIZE; j++) {
//			#pragma HLS PIPELINE II=1
//			#pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
//
//			uint512_dt tmpV1 = in1_re_buffer[j];
//			uint512_dt tmpV2 = in1_im_buffer[j];
//			uint512_dt tmpV3 = in2_re_buffer[j];
//			uint512_dt tmpV4 = in2_im_buffer[j];
//			uint512_dt tmpOut1 = 0;
//			uint512_dt tmpOut2 = 0;
//			din_type val1, val2, val3, val4;
//			dout_type res_re;
//			dout_type res_im;
//
//			parallel_add:
//			for (int i = 0; i < NUM_ELEMENTS; i++) {
//			   #pragma HLS UNROLL
//				val1 = tmpV1.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
//				val2 = tmpV2.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
//				val3 = tmpV3.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
//				val4 = tmpV4.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE);
//
//				res_re = ((val1 * val3) - (val2 * val4));
//				res_im = ((val2 * val3) + (val1 * val4));
//				tmpOut1.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE) = res_re;
//				tmpOut2.range(DATATYPE_SIZE * (i + 1) - 1, i * DATATYPE_SIZE) = res_im;
//			}
//			out_re_buffer[j] = tmpOut1;
//			out_im_buffer[j] = tmpOut2;
//		}
//
//        write1:
//		for (int j = 0; j < BUFFER_SIZE; j++) {
//			#pragma HLS PIPELINE II=1
//			#pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
//
//			out_re[i + j] = out_re_buffer[j];
//			out_im[i + j] = out_im_buffer[j];
//        }
    }
}
}


