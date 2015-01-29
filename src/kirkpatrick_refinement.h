#ifndef _KIRKPATRICK_REFINEMENT_H
#define _KIRKPATRICK_REFINEMENT_H

#include "graph.h"
#include "triangle.h"
#include "geom/primitives/contour.h"

#include <set>
#include <vector>
#include <deque>
#include <functional>

namespace geom
{
  namespace algorithms
  {
    namespace localization
    {
      using geom::structures::point_type;
      using geom::structures::contour_type;
      using geom::structures::graph_type;
      using geom::structures::triangle_type;

      struct kirkpatrick_refinement
      {
        typedef uint32_t id_type;
        const size_t DEGREE_THRESHOLD = 12;

        kirkpatrick_refinement(std::vector<point_type> const & poly);

        id_type find_query(point_type const & point) const;
        id_type find_step(point_type const & point, id_type from = 0) const;
        bool is_leaf(id_type) const;

        size_t triangles_num() const;

        triangle_type<point_type> triangle_by_id(id_type id) const;

        std::vector<point_type> const & points() const
        {
          return points_;
        }

        graph_type<triangle_type<id_type>> const & search_dag() const
        {
          return search_dag_;
        };

      private:
        std::vector<point_type> points_;
        graph_type<triangle_type<id_type>> search_dag_;

      private:
        typedef std::set<id_type> set_type;

        id_type add_triangle(triangle_type<id_type> const & t,
                             std::vector<set_type> & triangles);

        bool is_ear(id_type id1, id_type id2, id_type id3,
                    std::list<id_type> const & poly) const;

        std::vector<triangle_type<id_type>>
        triangulate(std::vector<id_type> const & poly) const;

        id_type next_point(id_type id, id_type t_id) const;

        std::vector<id_type>
        find_independent_set(std::deque<id_type> & from,
                             std::vector<set_type> const & triangles) const;
      };

    }
  }
}

#endif // _KIRKPATRICK_REFINEMENT_H
