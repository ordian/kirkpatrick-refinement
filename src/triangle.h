#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include <vector>

namespace geom {
namespace structures {

    template <typename Vertex>
    struct triangle_type {
        Vertex a, b, c;
        triangle_type(Vertex x, Vertex y, Vertex z): a(x), b(y), c(z) {}

        bool contains(point_type const& point) const;
        bool intersects(triangle_type<Vertex> const & other) const;

        std::vector<Vertex> to_vector() const {
            return std::vector<Vertex>({a, b, c});
        }
    };

}}

#endif // _TRIANGLE_H
