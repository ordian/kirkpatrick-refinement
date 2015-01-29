#ifndef _DETERMINANT_H
#define _DETERMINANT_H

#include "common.h"

inline int64_t det2(int64_t a, int64_t b,
                    int64_t c, int64_t d)
{
  return a * d - b * c;
}

inline int64_t det3(int64_t x11, int64_t x12, int64_t x13,
                    int64_t x21, int64_t x22, int64_t x23,
                    int64_t x31, int64_t x32, int64_t x33)
{
  return x11 * det2(x22, x23,
                    x32, x33)
      -  x12 * det2(x21, x23,
                    x31, x33)
      +  x13 * det2(x21, x22,
                    x31, x32);
}

#endif // _DETERMINANT_H
