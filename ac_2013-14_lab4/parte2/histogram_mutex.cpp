/* histogram.cpp */
#include <array>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "img_aux.hpp"
#define IMAGESIZE 256   /* = GRAYLEVEL */

std::array<long int, GRAYLEVEL> histogram;
long int max_frequency; /* Maximum frequency */
std::mutex histogram_mutex;
pthread_barrier_t barrier;

void make_histogram_image(int offset, int workers)
/* Histogram of image1 is output into image2 */
{
  int i, j, x, y, workload; /* control variable */
  int height; /* height of histogram */
  
  workload = y_size1 / workers;
  for (y = workload * offset; y < (offset+1)*workload ; y++) {
    for (x = 0; x < x_size1; x++) {
      histogram_mutex.lock();
      histogram[image1[y][x]]++;
      histogram_mutex.unlock();
    }
  }

  pthread_barrier_wait(&barrier);

  /* calculation of maximum frequency */
  workload = GRAYLEVEL / workers;
  for (i = workload * offset; i < (offset+1) * workload; i++) {
    histogram_mutex.lock();
    if(histogram[i] > max_frequency)
      max_frequency = histogram[i];
    histogram_mutex.unlock();
  }

  pthread_barrier_wait(&barrier);

  /* Histogram image generation */
  x_size2 = IMAGESIZE;
  y_size2 = IMAGESIZE;

  workload = GRAYLEVEL / workers;
  for (i = workload * offset; i < (offset+1) * workload; i++) {
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
  pthread_barrier_init(&barrier, NULL, n_threads);

  load_image_file(argv[1]);       	/* Input of image1 */

  for(i = 0; i < iter; i++)
  {
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



