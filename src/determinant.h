#ifndef _DETERMINANT_H
#define _DETERMINANT_H

inline int64_t det2(int64_t a, int64_t b,
                    int64_t c, int64_t d) {
    return a * d - b * c;
}

inline int64_t det3(int64_t x11, int64_t x12, int64_t x13,
                    int64_t x21, int64_t x22, int64_t x23,
                    int64_t x31, int64_t x32, int64_t x33) {
    return x11 * det2(x22, x23,
                      x32, x33)
        -  x12 * det2(x21, x23,
                      x31, x33)
        +  x13 * det2(x21, x22,
                      x31, x32);
}

int64_t det4(int64_t x11, int64_t x12, int64_t x13, int64_t x14,
             int64_t x21, int64_t x22, int64_t x23, int64_t x24,
             int64_t x31, int64_t x32, int64_t x33, int64_t x34,
             int64_t x41, int64_t x42, int64_t x43, int64_t x44) {
    return x11 * det3(x22, x23, x24,
                      x32, x33, x34,
                      x42, x43, x44)
        -  x12 * det3(x21, x23, x24,
                      x31, x33, x34,
                      x41, x43, x44)
        +  x13 * det3(x21, x22, x24,
                      x31, x32, x34,
                      x41, x42, x44)
        -  x14 * det3(x21, x22, x23,
                      x31, x32, x33,
                      x41, x42, x43);
}

#endif // _DETERMINANT_H
