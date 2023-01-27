#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
#include <assert.h>
#include <stdint.h>

extern void saxpySerial(int N,
			float scale,
			float X[],
			float Y[],
			float result[]);


void saxpyStreaming(int N,
                    float scale,
                    float X[],
                    float Y[],
                    float result[])
{
    // Replace this code with ones that make use of the streaming instructions
    __m128 vec_scale = _mm_broadcast_ss (&scale);
    long unsigned int vec_alignment  = 128;
    long unsigned int vec_width(vec_alignment/sizeof(float));
	long unsigned int nbVec = (N/vec_width);
    //saxpySerial(N, scale, X, Y, result);
    for (int i=0; i<nbVec; i++) {
        //result[i] = scale * X[i] + Y[i];
        __m128 vec_X = _mm_stream_load_si128(X + i*vec_width);
		__m128 vec_aX = _mm_mul_ps (vecX, vec_scale);
		__m128 vec_Y = _mm_stream_load_si128(Y + i*vec_width);
		__m128 vec_result = _mm_add_ps(vecAX, vecY);
		_mm_store_ps(result + i*vec_width, vec_result);
    }
}

