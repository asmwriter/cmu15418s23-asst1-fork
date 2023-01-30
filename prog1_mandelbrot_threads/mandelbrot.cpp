#include <stdio.h>
#include <pthread.h>

// Use this code to time your threads
#include "CycleTimer.h"


/*

  15418 Spring 2012 note: This code was modified from example code
  originally provided by Intel.  To comply with Intel's open source
  licensing agreement, their copyright is retained below.

  -----------------------------------------------------------------

  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// Core computation of Mandelbrot set membershop
// Iterate complex number c to determine whether it diverges
static inline int mandel(float c_re, float c_im, int count)
{
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i) {

        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re*z_re - z_im*z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }

    return i;
}

//
// MandelbrotSerial --
//
// Compute an image visualizing the mandelbrot set.  The resulting
// array contains the number of iterations required before the complex
// number corresponding to a pixel could be rejected from the set.
//
// * x0, y0, x1, y1 describe the complex coordinates mapping
//   into the image viewport.
// * width, height describe the size of the output image
// * startRow, endRow describe how much of the image to compute
void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int endRow,
    int maxIterations,
    int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    for (int j = startRow; j < endRow; j++) {
        for (int i = 0; i < width; ++i) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}



void mandelbrotSerial_v2(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int endRow,
    int maxIterations,
    int skipRows, 
    int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    for (int j = startRow; j < endRow; j=j+skipRows) {
        for (int i = 0; i < width; ++i) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}


// Struct for passing arguments to thread routine
typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;



//
// workerThreadStart --
//
// Thread entrypoint.
void* workerThreadStart(void* threadArgs) {
    double startTime = CycleTimer::currentSeconds();
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    /*
        Work Decomposition policy 1: Blocked image computation
        1. Each thread computes an (almost) equal portion of mandelbrot image. 
        2. Each thread is assigned a block of Consecutive rows in the image. 
    */
    //printf("Hello world from thread %d\n", args->threadId);
    //printf("args->numthreads = %u\n", args->numThreads);
    /*
    unsigned int threadRows = args->height/args->numThreads;
    unsigned int remRows = args->height - args->numThreads*(args->height/args->numThreads);
    unsigned int threadStartRow = ( args->threadId * (args->height/args->numThreads) ) 
                                        + ((args->threadId < remRows) ? args->threadId:(args->threadId!=0? remRows:0));
    unsigned int threadEndRow = threadStartRow + threadRows + ((args->threadId<remRows)?1:0);
    */
    //printf("startrow = %u, endrow = %u\n", threadStartRow, threadEndRow);

    /* 
    Work Decomposition policy 2: Interleaving row computation of threads. 
        1. Each threads computes the mandelbrot image in an interleaved fashion.
        2. Each thread i computes the image at rows: i, i+numThreads, i+2*numThreads and so on until the image is done.
        
    */
    int skipRows = args->numThreads;
    
    mandelbrotSerial_v2(args->x0, args->y0, args->x1, args->y1, 
                    args->width, (args->height), 
                    args->threadId, args->height, 
                    args->maxIterations, skipRows, args->output); 
    double endTime = CycleTimer::currentSeconds();
    return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    const static int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    pthread_t workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
        // TODO: Set thread arguments here.
        args[i].threadId = i;
        args[i].numThreads = numThreads;
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1; 
        args[i].width = width;
        args[i].height = height ; 
        args[i].maxIterations = maxIterations;
        args[i].output = output;
    }

    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=1; i<numThreads; i++)
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

    workerThreadStart(&args[0]);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);
}
