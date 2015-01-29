#include "kirkpatrick_refinement.h"
#include "turn.h"
#include "circular.h"

#include <algorithm>
#include <cassert>

namespace geom
{
  namespace algorithms
  {
    namespace localization
    {
      kirkpatrick_refinement::kirkpatrick_refinement(std::vector<point_type> const & poly)
        : points_(poly.begin(), poly.end())
      {
        // poly should be oriented counter clock wise
        assert(poly.size() > 2);
        const id_type n = poly.size();
        // DCEL would be a better alternative
        std::vector<set_type> triangles(n + 3);

        // rotate to leftmost
        auto leftmost = std::min_element(points_.begin(), points_.end());
        std::rotate(points_.begin(), leftmost, points_.end());

        // bounds
        auto rightmost = std::max_element(points_.begin(), points_.end());
        auto rightmost_id = rightmost - points_.begin();
        auto minmax_y = std::minmax_element(points_.begin(), points_.end(),
                                            [](point_type const & l,
                                               point_type const & r)
                                            {
                                              if (l.y == r.y)
                                                return l.x < r.x;
                                              return l.y < r.y;
                                            });
        auto downmost = *std::get<0>(minmax_y);
        auto upmost = *std::get<1>(minmax_y);
        point_type leftdown(points_[0].x, downmost.y);
        int margin = 73;
        // top triangle
        points_.emplace_back(leftdown.x - margin, leftdown.y - margin);
        points_.emplace_back((rightmost->x << 1) - leftdown.x + margin,
                             leftdown.y - margin);
        points_.emplace_back(leftdown.x - margin,
                             (upmost.y << 1) - leftdown.y + margin);
        add_triangle({n, n + 1, n + 2}, triangles);

        for (id_type i = 0; i < n; i++)
          assert(triangle_by_id(0).contains(points_[i]));

        // initial triangulation
        std::vector<id_type> lower_part = {n, n + 1}, upper_part;
        std::vector<id_type> initial(n);
        std::iota(initial.begin(), initial.end(), 0);

        for (auto i = rightmost_id; i > 0; --i)
          lower_part.push_back(i);
        lower_part.push_back(0);

        for (auto i = n - 1; i >= rightmost_id; --i)
          upper_part.push_back(i);
        upper_part.insert(upper_part.end(), {n + 1, n + 2, 0});

        for (auto triangle: triangulate(initial))
          add_triangle(triangle, triangles);

        add_triangle({n + 2, n, 0}, triangles);
        for (auto triangle: triangulate(lower_part))
          add_triangle(triangle, triangles);

        for (auto triangle: triangulate(upper_part))
          add_triangle(triangle, triangles);

        // low degree vertices
        std::deque<id_type> low_degree;
        for (id_type i = 0; i < n; ++i)
          if (triangles[i].size() < DEGREE_THRESHOLD)
            low_degree.push_back(i);

        // main loop
        while (true)
          {
            auto iset = find_independent_set(low_degree, triangles);
            if (iset.empty())
              break;
            for (id_type j: iset)
              {
                assert(j < n);
                auto & jth = triangles[j];
                std::vector<id_type> adjacent_triangles(jth.begin(), jth.end());
                assert(adjacent_triangles.size() >= 3);
                // remove neighbour triangles and construct poly
                jth.clear();
                std::vector<id_type> points(adjacent_triangles.size());
                std::transform(adjacent_triangles.begin(), adjacent_triangles.end(),
                               points.begin(), [&](id_type t)
                {
                  return next_point(j, t);
                });
                // sort triangles
                std::sort(points.begin(), points.end(), [&](id_type l, id_type r)
                          {
                            double angleL = std::atan2(points_[l].y - points_[j].y,
                                                       points_[l].x - points_[j].x);
                            double angleR = std::atan2(points_[r].y - points_[j].y,
                                                       points_[r].x - points_[j].x);
                            return angleL < angleR;
                          });
                // memorize degrees
                std::vector<size_t> neighbours_degrees(points.size());
                std::transform(points.begin(), points.end(),
                               neighbours_degrees.begin(),
                               [&](id_type id) {return triangles[id].size();});
                // remove neighbour triangles
                for (id_type t: adjacent_triangles)
                  {
                    id_type p = next_point(j, t);
                    for (auto neighbour: {p, next_point(p, t)})
                      {
                        auto found = triangles[neighbour].find(t);
                        assert(found != triangles[neighbour].end());
                        triangles[neighbour].erase(found);
                      }
                  }

                // re triangulate
                auto triangulation = triangulate(points);
                for (auto triangle: triangulation)
                  {
                    size_t size = (triangle == search_dag_.vertices[0])
                      ? 0
                      : add_triangle(triangle, triangles);
                    // update search dag
                    for (auto i: adjacent_triangles)
                      {
                        auto old_triangle = triangle_by_id(i);
                        auto new_triangle = triangle_by_id(size);
                        if (old_triangle.intersects(new_triangle))
                          search_dag_.add_edge(size, i);
                      }
                  }

                // add new low degree points
                for (size_t i = 0; i < points.size(); ++i)
                  if (neighbours_degrees[i] >= DEGREE_THRESHOLD &&      // before
                      triangles[points[i]].size() < DEGREE_THRESHOLD && // after
                      points[i] < n)                                    // not root
                    low_degree.push_back(points[i]);
              }

            // filter low_degree
            auto it = std::remove_if(low_degree.begin(),
                                     low_degree.end(),
                                     [&](id_type id)
                                     {
                                       return triangles[id].size() >= DEGREE_THRESHOLD;
                                     });
            if (it != low_degree.end())
              low_degree.erase(it, low_degree.end());
          }
      }

      kirkpatrick_refinement::id_type
      kirkpatrick_refinement::find_query(point_type const & point) const
      {
        id_type id = 0;
        while (!is_leaf(id))
          id = find_step(point, id);
        return id;
      }

      kirkpatrick_refinement::id_type
      kirkpatrick_refinement::find_step(point_type const & point, id_type from) const
      {
        for (id_type id: search_dag_.edges[from])
          if (triangle_by_id(id).contains(point))
            return id;
        return from;
      }

      bool kirkpatrick_refinement::is_leaf(id_type id) const
      {
        assert(id < triangles_num());
        return search_dag_.edges[id].empty();
      }

      triangle_type<point_type>
      kirkpatrick_refinement::triangle_by_id(id_type id) const
      {
        assert(id < triangles_num());
        auto const & t = search_dag_.vertices[id];
        return triangle_type<point_type>(points_[t.a],
                                         points_[t.b],
                                         points_[t.c]);
      }

      size_t kirkpatrick_refinement::triangles_num() const
      {
        return search_dag_.vertices.size();
      }

      kirkpatrick_refinement::id_type
      kirkpatrick_refinement::add_triangle(triangle_type<id_type> const & t,
                                           std::vector<set_type> & triangles)
      {
        size_t size = search_dag_.vertices.size();
        search_dag_.vertices.push_back(t);
        for (id_type id: t.to_vector())
          triangles[id].insert(size);
        return size;
      }

      bool
      kirkpatrick_refinement::is_ear(id_type id1,
                                     id_type id2,
                                     id_type id3,
                                     std::list<id_type> const & poly) const
      {
        if (!is_left_turn(points_[id1], points_[id2], points_[id3]))
          return false;

        for (id_type id: poly) {
          if (id == id1 || id == id2 || id == id3)
            continue;
          if (triangle_type<point_type>(points_[id1],
                                        points_[id2],
                                        points_[id3])
              .contains(points_[id]))
            return false;
        }
        return true;
      }

      std::vector<triangle_type<kirkpatrick_refinement::id_type>>
      kirkpatrick_refinement::triangulate(std::vector<id_type> const & poly) const
      {
        // ear clipping
        std::vector<triangle_type<id_type>> result;
        assert(poly.size() >= 3);
        std::list<id_type> dcvl(poly.begin(), poly.end());
        std::list<id_type>::iterator v = dcvl.begin();
        std::advance(v, rand() % dcvl.size());

        while (dcvl.size() != 3)
          {
            auto v_prev = ::prev(v, dcvl);
            auto v_next = ::next(v, dcvl);
            if (is_ear(*v_prev, *v, *v_next, dcvl))
              {
                result.emplace_back(*v_prev, *v, *v_next);
                dcvl.erase(v);
              }
            v = v_next;
          }

        result.emplace_back(*dcvl.begin(),
                            *(std::next(dcvl.begin())),
                            *(std::prev(dcvl.end())));
        return result;
      }

      kirkpatrick_refinement::id_type
      kirkpatrick_refinement::next_point(id_type id, id_type t_id) const
      {
        auto const & t = search_dag_.vertices[t_id];
        id_type result = t.a;
        if (id == t.a)
          result = t.b;
        else if (id == t.b)
          result = t.c;
        else
          assert(id == t.c);
        return result;
      }

      std::vector<kirkpatrick_refinement::id_type>
      kirkpatrick_refinement::find_independent_set(std::deque<id_type> & from,
                                                   std::vector<set_type> const & triangles) const
      {
        size_t size = from.size();
        std::vector<id_type> result;
        std::set<id_type> forbidden;

        id_type j = from.front();
        for (size_t i = 0; i < size; ++i)
          {
            assert(triangles[j].size() < DEGREE_THRESHOLD);
            from.pop_front();
            if (forbidden.find(j) == forbidden.end())
              {
                result.push_back(j);
                for (auto i: triangles[j])
                  {
                    auto p = next_point(j, i);
                    forbidden.insert(p);
                    forbidden.insert(next_point(p, i));
                  }
              }
            else
              from.push_back(j);
            j = from.front();
          }

        return result;
      }

    }
  }
}
