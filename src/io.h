#ifndef _IO_H
#define _IO_H

#include <ostream>
#include "homogeneous.h"
#include "triangle.h"


namespace geom {
namespace structures {

    std::ostream & operator << (std::ostream & out, homogeneous_point_type const & p);
    template <typename T>
    std::ostream & operator << (std::ostream & out, triangle_type<T> const & t);

}}


#endif // _IO_H
