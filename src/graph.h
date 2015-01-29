#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include "common.h"

namespace geom
{
  namespace structures
  {
    template<typename Vertex>
    struct graph_type
    {
      std::vector<Vertex> vertices;
      std::vector<std::vector<id_type>> edges;

      void add_edge(id_type from, id_type to)
      {
        if (edges.size() < std::max(from, to) + 1)
          edges.resize(std::max(from, to) + 1);
        edges[from].push_back(to);
      }
    };
  }
}

#endif // _GRAPH_H
