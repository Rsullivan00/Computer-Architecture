#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "img_aux.hpp"

unsigned char image1[MAX_IMAGESIZE][MAX_IMAGESIZE],
  image2[MAX_IMAGESIZE][MAX_IMAGESIZE];
int x_size1, y_size1, /* width & height of image1*/
  x_size2, y_size2; /* width & height of image2 */

void load_image_file(char *filename)
/* Input of header & body information of pgm file */
/* for image1[ ][ ]Cx_size1Cy_size1 */
{
  std::ifstream file(filename, std::ios::in);
  if(file.is_open()) {
    /* char buffer[MAX_BUFFERSIZE]; */
    std::string buffer;
    int max_gray, x, y;

    file >> buffer;
    if (buffer[0] != 'P' || buffer[1] != '5') {
      std::cout << "     Mistaken file format, not P5!" << std::endl << std::endl;
      file.close();
      exit(1);
    }

    x_size1 = 0;
    y_size1 = 0;
    while (x_size1 == 0 || y_size1 == 0) {
      file >> buffer;
      /* file.read(buffer, MAX_BUFFERSIZE); */
      if (buffer[0] >= '0' && buffer[0] <= '9') {
        sscanf(buffer.c_str(), "%d", &x_size1);
        file >> buffer;
        sscanf(buffer.c_str(), "%d", &y_size1);
      }
    }
    /* input of max_gray */
    max_gray = 0;
    while (max_gray == 0) {
      file >> buffer;
      if (buffer[0] >= '0' && buffer[0] <= '9') {
        sscanf(buffer.c_str(), "%d", &max_gray);
      }
    }
    if (x_size1 > MAX_IMAGESIZE || y_size1 > MAX_IMAGESIZE) {
      std::cout << "     Image size exceeds " << MAX_IMAGESIZE << " x " << MAX_IMAGESIZE << std::endl << std::endl;
      std::cout << "     Please use smaller images!" << std::endl << std::endl;
      file.close();
      exit(1);
    }
    if (max_gray != MAX_BRIGHTNESS) {
      std::cout << "     Invalid value of maximum gray level!" << std::endl << std::endl;
      file.close();
      exit(1);
    }
    /* Input of image data*/
    for (y = 0; y < y_size1; y++) {
      for (x = 0; x < x_size1; x++) {
        image1[y][x] = (unsigned char)file.get();
      }
    }

    file.close();

  } else std::cout << "The file couldn't be opened" << std::endl;


}

void save_image_file(char *filename)
/* Output of image2[ ][ ], x_size2, y_size2 */ 
/* into pgm file with header & body information */
{
  int y, x;

  std::ofstream file;
  file.open(filename);

  file << "P5" << std::endl;
  file << "# Created by Image Processing" << std::endl;
  file << x_size2 << " " << y_size2 << std::endl;
  file << MAX_BRIGHTNESS << std::endl;

  for (y = 0; y < y_size2; y++) {
    for (x = 0; x < x_size2; x++) {
      file << image2[y][x];
    }
  }
}
