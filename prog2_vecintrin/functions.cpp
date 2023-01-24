#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;


void absSerial(float* values, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	if (x < 0) {
	    output[i] = -x;
	} else {
	    output[i] = x;
	}
    }
}

// implementation of absolute value using 15418 instrinsics
void absVector(float* values, float* output, int N) {
    __cmu418_vec_float x;
    __cmu418_vec_float result;
    __cmu418_vec_float zero = _cmu418_vset_float(0.f);
    __cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

    //  Note: Take a careful look at this loop indexing.  This example
    //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
    //  Why is that the case?
    for (int i=0; i<N; i+=VECTOR_WIDTH) {

	// All ones
	maskAll = _cmu418_init_ones();

	// All zeros
	maskIsNegative = _cmu418_init_ones(0);

	// Load vector of values from contiguous memory addresses
	_cmu418_vload_float(x, values+i, maskAll);               // x = values[i];

	// Set mask according to predicate
	_cmu418_vlt_float(maskIsNegative, x, zero, maskAll);     // if (x < 0) {

	// Execute instruction using mask ("if" clause)
	_cmu418_vsub_float(result, zero, x, maskIsNegative);      //   output[i] = -x;

	// Inverse maskIsNegative to generate "else" mask
	maskIsNotNegative = _cmu418_mask_not(maskIsNegative);     // } else {

	// Execute instruction ("else" clause)
	_cmu418_vload_float(result, values+i, maskIsNotNegative); //   output[i] = x; }

	// Write results back to memory
	_cmu418_vstore_float(output+i, result, maskAll);
    }
}

// Accepts an array of values and an array of exponents
// For each element, compute values[i]^exponents[i] and clamp value to
// 4.18.  Store result in outputs.
// Uses iterative squaring, so that total iterations is proportional
// to the log_2 of the exponent
void clampedExpSerial(float* values, int* exponents, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	float result = 1.f;
	int y = exponents[i];
	float xpower = x;
	while (y > 0) {
	    if (y & 0x1) {
			result *= xpower;
		}
	    xpower = xpower * xpower;
	    y >>= 1;
	}
	if (result > 4.18f) {
	    result = 4.18f;
	}
	output[i] = result;
    }
}

void printVec(const __cmu418_vec_float &x) {
	for (int i = 0; i < VECTOR_WIDTH; i++) {
		std::cout << x.value[i] << ", ";
	}
	std::cout << '\n';
}

void printVec_Int(const __cmu418_vec_int &x) {
	for (int i = 0; i < VECTOR_WIDTH; i++) {
		std::cout << x.value[i] << ", ";
	}
	std::cout << '\n';
}

void clampedExpVector(float* values, int* exponents, float* output, int N) {
    // Implement your vectorized version of clampedExpSerial here
    //  ...
	__cmu418_vec_float x, result, const1, xpower;
	__cmu418_vec_int vectOnes, vectZeroes, y;	
	__cmu418_mask maskAll, ydonemask, ylsbmask, resultgt;
	

	for (int i=0; i<N; i+=VECTOR_WIDTH) {
		
		// All ones
		maskAll = _cmu418_init_ones();
		
		// Load vector of values from contiguous memory addresses
		//x = values[i]
		_cmu418_vload_float(x, values+i, maskAll);               

		//Set vector result to 1.0f
		//result = 1.f
		_cmu418_vset_float(result, 1.f, maskAll);
		//constant = 4.18f
		_cmu418_vset_float(const1, 4.18f, maskAll);

		_cmu418_vset_int(vectOnes, 1, maskAll);
		_cmu418_vset_int(vectZeroes, 0, maskAll);
		std::cout<<"printing vectOnes"<<std::endl;
		printVec_Int(vectOnes);
		std::cout<<"printing vectZeroes"<<std::endl;
		printVec_Int(vectZeroes);
		
		//All ones and zero integers
		

		//Load vector of values from exponents array
		//y = exponents[i]
		_cmu418_vload_int(y, exponents+i, maskAll);
		
		//move vector x to xpower
		//float xpower = x
		_cmu418_vmove_float(xpower, x, maskAll);
		
		int done = 0;
		__cmu418_mask ydonemask = _cmu418_init_ones();
		//Compute y>0
		_cmu418_vgt_int(ydonemask, y, vectZeroes, maskAll);
		done = _cmu418_cntbits(ydonemask);
		// y > 0
		while(done){
			// Compute y & 0x1
			__cmu418_vec_int ylsb;
			_cmu418_vbitand_int(ylsb, y, vectOnes, ydonemask);
			
			//Generate mask for if(y & 0x1)
			__cmu418_mask ylsbmask;
			_cmu418_veq_int(ylsbmask, ylsb, vectOnes, ydonemask);
			
			//Compute result *= xpower with y&0x1 mask
			_cmu418_vmult_float(result, xpower, xpower, ylsbmask);
		
			//Compute xpower = xpower * xpower with ydonemask
			_cmu418_vmult_float(xpower, xpower, xpower, ydonemask);
			
			// Compute y>> = 1 with ydonemask
			_cmu418_vshiftright_int(y, y, vectOnes, ydonemask);
			
			//Compute y>0
			_cmu418_vgt_int(ydonemask, y, vectZeroes, maskAll);

			done = _cmu418_cntbits(ydonemask);
		}
		
		//Compute result > 4.18f for all lanes
		_cmu418_vgt_float(resultgt, result, const1, maskAll);

		//Set result = 4.18f for lanes 
		_cmu418_vset_float(result, 4.18f, resultgt);
		
		//output[i] = result
		_cmu418_vstore_float(output+i, result, maskAll);

	}
}


float arraySumSerial(float* values, int N) {
    float sum = 0;
    for (int i=0; i<N; i++) {
	sum += values[i];
    }

    return sum;
}

// Assume N % VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float* values, int N) {
    // Implement your vectorized version here
    //  ...
	int level = 0;
	float sum = 0.0f;
	int stride = VECTOR_WIDTH;
	__cmu418_mask maskAll = _cmu418_init_ones();
	__cmu418_vec_float op_add1, op_add2;
	
	for(int stride = VECTOR_WIDTH; stride<N; stride = stride<<1){
		for (int i=0; i<N; i+=stride*2) {
			_cmu418_vload_float(op_add1, values+i, maskAll); 
			_cmu418_vload_float(op_add2, values+i+stride, maskAll); 
			_cmu418_vadd_float(op_add1, op_add1, op_add2, maskAll);
			_cmu418_vstore_float(values+i, op_add1, maskAll);
		}
	}
	for(int i = 0; i<VECTOR_WIDTH; i++){
		sum += values[i];
	}
	return sum;
}
