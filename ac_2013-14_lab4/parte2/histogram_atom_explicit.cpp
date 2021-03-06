/* histogram.cpp 
   Rick Sullivan and Zuza Brzeska
   12 December 2013
*/
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
      /* Now here, instead of using mutexes, we can substitute an atomic instruction to increment the histogram array. */
      /* Since we are only incrementing the histogram array (and synchronizing below), we can relax ordering. */
      char_count = std::atomic_fetch_add_explicit(&histogram[image1[y][x]], (long)1, std::memory_order_seq_cst) + 1; /* Atomic increment */
      prev_max_frequency = std::atomic_load_explicit(&max_frequency, std::memory_order_seq_cst);
      while (char_count > prev_max_frequency) {
        /* max_frequency will always have the maximum value. Therefore, if it changes
         while we are updating it, we must check again if we should
         update it once more. */
        if (std::atomic_compare_exchange_strong_explicit(&max_frequency, 
            &prev_max_frequency, char_count, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
          /* If we updated the max_frequency, we will now need to recalculate the image2 outputs 
              for every pixel. */
          for (i = 0; i < IMAGESIZE; i++) {
            height = (int)(MAX_BRIGHTNESS / (double)char_count * (double)std::atomic_load_explicit(&histogram[i], std::memory_order_seq_cst));
            for (j = 0; j < IMAGESIZE; j++) {
              image2[IMAGESIZE-1-j][i] = height;
            }
          }
        }
      }

      /* Whenever we add to a count in the histogram array, we must update the corresponding image2 output */
      image2[x][y] = (int)(MAX_BRIGHTNESS / (double)std::atomic_load_explicit(&max_frequency, std::memory_order_seq_cst) * (double)char_count);
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


  /* Histogram image generation */
  x_size2 = IMAGESIZE;
  y_size2 = IMAGESIZE;

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

  std::cout << "Done" << std::endl;
  save_image_file(argv[2]);       	/* Output of image2 */

  std::cout << "histogram takes: " << (double)usec / 1000000 << "sec    mean: " << (double)usec / 1000000 / iter <<"sec    image size: " << y_size1*x_size1 << std::endl;
  return 0;
}



