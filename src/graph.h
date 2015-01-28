#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include <deque>

namespace geom {
namespace structures {

template<typename Vertex>
struct graph_type {
    std::vector<Vertex> vertices;
    std::vector<std::deque<uint32_t>> edges;
};

}}

#endif // _GRAPH_H
