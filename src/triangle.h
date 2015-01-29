#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include <vector>
#include "geom/primitives/point.h"

namespace geom
{
  namespace structures
  {
    template <typename Vertex>
    struct triangle_type
    {
      Vertex a, b, c;
      triangle_type(Vertex x, Vertex y, Vertex z): a(x), b(y), c(z) {}

      bool contains(point_type const& point) const;
      bool intersects(triangle_type<Vertex> const & other) const;

      bool operator ==(triangle_type<Vertex> const & that) const
      {
        return (a == that.a && b == that.b && c == that.c)
          || (b == that.a && c == that.b && a == that.c)
          || (c == that.a && a == that.b && b == that.c);
      }

      bool operator !=(triangle_type<Vertex> const & that) const
      {
        return !(this == that);
      }

      std::vector<Vertex> to_vector() const
      {
        return std::vector<Vertex>({a, b, c});
      }
    };

  }
}

#endif // _TRIANGLE_H
