#include <ostream>
#include "homogeneous.h"
#include "triangle.h"


namespace geom {
namespace structures {

    std::ostream & operator << (std::ostream & out, homogeneous_point_type const & p) {
        out << "(" << p.x << ", " << p.y << ", " << p.omega << ")";
        return out;
    }

    template <typename T>
    std::ostream & operator << (std::ostream & out, triangle_type<T> const & t) {
        out << "<" << t.a << "; " << t.b << "; " << t.c << ">";
        return out;
    }

}}
