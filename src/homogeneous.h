#ifndef _HOMOGENEOUS_H
#define _HOMOGENEOUS_H

#include "geom/primitives/point.h"

namespace geom {
namespace structures {

    struct homogeneous_point_type: point_type {
        int32_t omega = 1;
        homogeneous_point_type(): point_type() {}
        homogeneous_point_type(point_type const & base): point_type(base) {}
        homogeneous_point_type(int32_t x, int32_t y, int32_t z): point_type(x, y), omega(z) {}
        explicit operator point_type() const { return (point_type) *this; }
        bool operator==(homogeneous_point_type const & that) const {
            return x == that.x && y == that.y && omega == that.omega;
        }
        bool operator!=(homogeneous_point_type const & that) const {
            return !(*this == that);
        }
    };

}}

#endif // _HOMOGENEOUS_H
