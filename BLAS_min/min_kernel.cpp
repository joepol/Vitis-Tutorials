#include "ap_int.h"
#include <hls_stream.h>
#include "xf_blas.hpp"
//#include "xf_blas/helpers/utils/types.hpp"

//#define BUFFER_SIZE 	1024 * 1024	//required buffer size to be used in order to work on local data
#define DATAWIDTH 		128	//data width to be used in the gmem
#define DATATYPE_SIZE 	32		//int
#define NUM_ELEMENTS 	(DATAWIDTH / DATATYPE_SIZE) // number of elements in one type of DATAWIDTH 128/32 = 4
#define TOTAL_NUM_ELEMENTS BUFFER_SIZE * 4

//#define BLAS_dataType int32_t
//#define BLAS_logParEntries 2

typedef ap_uint<DATAWIDTH> uint128_dt;
typedef ap_uint<DATATYPE_SIZE> din_type;
typedef ap_uint<DATATYPE_SIZE + 1> dout_type;

//TRIPCOUNT identifier
//const unsigned int c_chunk_sz = BUFFER_SIZE;
//const unsigned int c_size = NUM_ELEMENTS;
using namespace xf::blas;

extern "C" {

void min_kernel(int32_t* inVec,
				int* resultIndex, // index of the minimal item in the vector
				int p_n
		)
{

#pragma HLS INTERFACE m_axi port = inVec offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = resultIndex offset = slave bundle = gmem

#pragma HLS INTERFACE s_axilite port = inVec bundle = control
#pragma HLS INTERFACE s_axilite port = resultIndex bundle = control
#pragma HLS INTERFACE s_axilite port = p_n bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

//#pragma HLS STREAM variable = inVecStream //depth = 1024


//	void uut_top(uint32_t p_n,
//	             BLAS_dataType p_alpha,
//	             BLAS_dataType p_x[BLAS_vectorSize],
//	             BLAS_dataType p_y[BLAS_vectorSize],
//	             BLAS_dataType p_xRes[BLAS_vectorSize],
//	             BLAS_dataType p_yRes[BLAS_vectorSize],
//	             BLAS_resDataType& p_goldRes) {
	    int res;

	    hls::stream<WideType<int32_t, 1 << 2> > l_str;
	#pragma HLS data_pack variable = l_str
	#pragma HLS DATAFLOW
	    readVec2Stream<int32_t, 1 << 2>(inVec, p_n, l_str);
	    amin<int32_t, 2, int>(p_n, l_str, res);
	    *resultIndex = res;

}
}

