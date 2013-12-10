/* histogram.cpp */
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "img_aux.hpp"
#define IMAGESIZE 256   /* = GRAYLEVEL */

std::array<std::atomic<long>, GRAYLEVEL> histogram; /* Array of atomic long int variables */
std::atomic<long> max_frequency; /* Maximum frequency */
std::atomic<int> barrier1, barrier2;

void make_histogram_image(int offset, int workers)
/* Histogram of image1 is output into image2 */
{
  int i, j, x, y, workload; /* control variable */
  int height; /* height of histogram */
  long char_count, prev_max_frequency;

  workload = y_size1 / workers;
  for (y = workload * offset; y < (offset+1)*workload ; y++) {
    for (x = 0; x < x_size1; x++) {
      /* Now here, instead of using mutexes, we can substitute in atomic instructions */
      std::atomic_fetch_add(&histogram[image1[y][x]], (long)1); /* Atomic increment */
    }
  }

  /* We must synchronize here so that all frequency counting is completed before we
     determine the maximum frequency */
  std::atomic_fetch_add(&barrier1, 1);

  /* Spin lock until all workers reach this barrier */
  while (std::atomic_load(&barrier1) < workers) {
    ;
  }

  /* calculation of maximum frequency */
  workload = GRAYLEVEL / workers;
  for (i = workload * offset; i < (offset+1) * workload; i++) { 
    /* Note: histogram is no longer changing, so we do not need an atomic load */
    char_count = histogram[i];
    prev_max_frequency = std::atomic_load(&max_frequency);
    while (char_count > prev_max_frequency) {
      /* max_frequency will always have the maximum value. Therefore, if it changes
       while we are updating it, we must check again if we should
       update it once more. */
      std::atomic_compare_exchange_strong(&max_frequency, &prev_max_frequency, char_count);
    }
  }

  /* We need a barrier here so that processing is completely finished before
     we generate the 2nd image */
  std::atomic_fetch_add(&barrier2, 1);

  /* Spin lock until all workers reach this barrier */
  while (std::atomic_load(&barrier2) < workers) {
    ;
  }

  /* Histogram image generation */
  x_size2 = IMAGESIZE;
  y_size2 = IMAGESIZE;

  workload = GRAYLEVEL / workers;
  for (i = workload * offset; i < (offset+1) * workload; i++) {
    /* max_frequency and histogram are now only being read, so we do not need atomic loads */
    height = (int)(MAX_BRIGHTNESS / 
       (double)max_frequency * (double)histogram[i]);
    for (j = 0; j < height; j++) {
      image2[IMAGESIZE-1-j][i] = height;/* MAX_BRIGHTNESS; */
    }
  }
}


int main(int argc, char* argv[])
{
  if (argc < 5) {
        std::cout << "Usage:" << std::endl;
	std::cout << "   " << argv[0] << " <input_file> <output_file> <number of runs> <number of threads>" << std::endl;
        return 0;
  }


  struct timeval start, end;
  int iter = atoi(argv[3]), i, j, k;
  int n_threads = atoi(argv[4]);
  double usec = 0;
  std::vector<std::thread> threads;

  load_image_file(argv[1]);       	/* Input of image1 */

  for(i = 0; i < iter; i++)
  {
      barrier1 = barrier2 = 0;  /* Init barriers on every iteration */
      gettimeofday(&start, NULL);
      /* Calculation of histogram */
      for (j = 0; j < GRAYLEVEL; j++) {
        histogram[j] = 0;
      }
      max_frequency = -1;
      for (j = 0; j < IMAGESIZE ; j++) {
        for (k = 0; k < IMAGESIZE; k++) {
          image2[j][k] = 0;
        }
      }

      for (j = 0; j < n_threads; j++) {
        threads.push_back(std::thread(make_histogram_image, j, n_threads));
      }

      for (j = 0; j < n_threads; j++) {
        threads[j].join();
      }

      gettimeofday(&end, NULL); 
      usec += end.tv_usec + 1000000*(end.tv_sec - start.tv_sec) - start.tv_usec;

      threads.clear();
  }

  save_image_file(argv[2]);       	/* Output of image2 */

  std::cout << "histogram takes: " << (double)usec / 1000000 << "sec    mean: " << (double)usec / 1000000 / iter <<"sec    image size: " << y_size1*x_size1 << std::endl;
  return 0;
}



