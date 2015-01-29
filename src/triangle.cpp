#include "turn.h"
#include "triangle.h"
#include "common.h"
#include "geom/primitives/segment.h"

template <typename T> int sign(T val)
{
  return (T(0) < val) - (val < T(0));
}

namespace geom
{
  namespace structures
  {
    template <>
    bool triangle_type<point_type>::contains(point_type const & p) const
    {
      return turn(a, b, p) >= 0
          && turn(b, c, p) >= 0
          && turn(c, a, p) >= 0;
    }

    bool intersects(segment_type const & l, segment_type const & r)
    {
      int r0 = ::sign(turn(l[0], l[1], r[0]));
      int r1 = ::sign(turn(l[0], l[1], r[1]));
      int l0 = ::sign(turn(r[0], r[1], l[0]));
      int l1 = ::sign(turn(r[0], r[1], l[1]));
      return r0 * r1 == -1 && l0 * l1 == -1;
    }

    template <>
    bool triangle_type<point_type>::intersects(triangle_type<point_type> const & other) const
    {
      std::vector<segment_type> thiz, that;
      thiz.emplace_back(a, b); that.emplace_back(other.a, other.b);
      thiz.emplace_back(b, c); that.emplace_back(other.b, other.c);
      thiz.emplace_back(c, a); that.emplace_back(other.c, other.a);

      for (auto const & l: thiz)
        for (auto const & r: that)
          if (geom::structures::intersects(l, r))
            return true;

      bool a_in = other.contains(a);
      bool b_in = other.contains(b);
      bool c_in = other.contains(c);

      return a_in == b_in && b_in == c_in;
    }
  }
}
