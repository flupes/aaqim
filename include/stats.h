#ifndef AAQIM_STATS_H
#define AAQIM_STATS_H

#include <math.h>
#include <stdlib.h>

template <class T>
void mean_error(size_t size, const T data[], T &mean, float &nmae) {
  T sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += data[i];
  }
  mean = sum / static_cast<T>(size);
  float error = 0.0;
  for (size_t i = 0; i < size; i++) {
    error += fabs(data[i] - mean);
  }
  nmae = error / (float)(sum);
}

#endif
