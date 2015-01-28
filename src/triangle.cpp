#include "homogeneous.h"
#include "turn.h"
#include "triangle.h"
#include "geom/primitives/segment.h"


namespace geom {
namespace structures {

    template <>
    bool triangle_type<homogeneous_point_type>::contains(point_type const & p) const {
        auto w = homogeneous_point_type(p);

        bool ab = turn(a, b, w) >= 0;
        bool bc = turn(b, c, w) >= 0;
        bool ca = turn(c, a, w) >= 0;

        if (a.omega == 0)
          return !bc;
        else if (b.omega == 0)
          return !ca;
        else if (c.omega == 0)
          return !ab;

        return ab && bc && ca;
    }

    bool intersects(segment_type const & l, segment_type const & r) {
        int r0 = turn(l[0], l[1], r[0]); if (r0 == 0) return true;
        int r1 = turn(l[0], l[1], r[1]); if (r1 == 0) return true;
        int l0 = turn(r[0], r[1], l[0]); if (l0 == 0) return true;
        int l1 = turn(r[0], r[1], l[1]); if (l1 == 0) return true;
        return r0 != r1 && l0 != l1;
    }

    template <>
    bool triangle_type<homogeneous_point_type>::intersects(triangle_type<homogeneous_point_type> const & other) const {
        std::vector<segment_type> thiz, that;
        thiz.emplace_back(a, b); that.emplace_back(other.a, other.b);
        thiz.emplace_back(b, c); that.emplace_back(other.b, other.c);
        thiz.emplace_back(c, a); that.emplace_back(other.c, other.a);
        for (auto const & l: thiz)
            for (auto const & r: that)
                if (geom::structures::intersects(l, r))
                    return true;

        bool i1 = other.contains(a);
        bool i2 = other.contains(b);
        bool i3 = other.contains(c);

        return i1 == i2 && i2 == i3;
    }

}}
