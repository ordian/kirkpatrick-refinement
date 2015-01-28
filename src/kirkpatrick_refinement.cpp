#include "homogeneous.h"
#include "io/point.h"
#include "kirkpatrick_refinement.h"
#include "io.h"

#include <algorithm>
#include <cassert>
#include <list>

using geom::structures::homogeneous_point_type;

bool is_left_turn(homogeneous_point_type const & p,
                  homogeneous_point_type const & q,
                  homogeneous_point_type const & r);

template <typename T>
typename std::list<T>::iterator
next(typename std::list<T>::iterator iter,
     std::list<T> & container)
{
  auto result = std::next(iter);
  if (result == container.end())
    return container.begin();
  return result;
}

template <typename T>
typename std::list<T>::iterator
prev(typename std::list<T>::iterator iter,
     std::list<T> & container)
{
  if (iter == container.begin())
    return std::prev(container.end());
  return std::prev(iter);
}

namespace geom {
  namespace algorithms {
    namespace localization {

      kirkpatrick_refinement::kirkpatrick_refinement(std::vector<point_type> const & poly)
        : points_(poly.begin(), poly.end())
      {
        // poly should be oriented counter clock wise
        assert(poly.size() > 2);
        const size_t n = poly.size();
        // DCEL would be a better alternative
        std::vector<set_type> triangles(n + 1);
        // helper functions
        auto is_infinite = [&](triangle_type<id_type> const& t)
        {
          return t.a == n || t.b == n || t.c == n;
        };
        auto find_triangle = [&](id_type a,
            id_type b,
            id_type c)
        {
          auto found = std::find_if(search_dag_.vertices.begin(),
                                    search_dag_.vertices.end(),
                                    [&](triangle_type<id_type> const & t)
          {
              return (t.c == c && t.a == a && t.b == b)
              || (t.a == c && t.b == a && t.c == b)
              || (t.b == c && t.c == a && t.a == b);
        });
          return found - search_dag_.vertices.begin();
        };

        auto root = homogeneous_point_type(0, 0, 0);
        points_.push_back(root);
        // fake root triangle
        search_dag_.vertices.emplace_back(n, n, n);

        // initial triangulation
        std::vector<id_type> initial(n);
        std::iota(initial.begin(), initial.end(), 0);
        auto triangulation = triangulate(initial);
        for (auto triangle: triangulation)
          add_triangle(triangle, triangles);

        // triangles from root
        for (id_type i = 0; i < n; ++i)
          {
            id_type j = (i == n - 1) ? 0 : i + 1;
            auto t = triangle_type<id_type>(i, j, n);
            add_triangle(t, triangles);
          }

        // low degree vertices
        std::deque<id_type> low_degree;
        for (id_type i = 0; i < n; ++i)
          if (triangles[i].size() < DEGREE_THRESHOLD)
            low_degree.push_back(i);

        // main loop
        while (low_degree.size() > 3)
          {
            auto iset = find_independent_set(low_degree, triangles);
            bool is_last_iteration = low_degree.size() == 3;
            if (is_last_iteration)
              {
                if (!is_left_turn(points_[low_degree[0]],
                                  points_[low_degree[1]],
                                  points_[low_degree[2]]))
                  std::swap(low_degree[1], low_degree[2]);
              }
            for (size_t idx = 0; idx < iset.size(); idx++)
              {
                id_type j = iset[idx];
                assert (j != n);
                auto & jth = triangles[j];
                std::vector<id_type> adjacent_triangles(jth.begin(), jth.end());
                assert(adjacent_triangles.size() > 2);
                // remove neighbour triangles and construct poly
                jth.clear();
                std::vector<id_type> points;
                id_type first = 0;
                for (id_type t: adjacent_triangles)
                  {
                    id_type p = next_point(j, t);
                    if (next_point(p, t) == n)
                      first = p;
                    else if (p != n)
                      points.push_back(p);
                    else
                      points.push_back(next_point(n, t));
                    for (auto neighbour: {p, next_point(p, t)})
                      {
                        auto found = triangles[neighbour].find(t);
                        assert(found != triangles[neighbour].end());
                        triangles[neighbour].erase(found);
                      }
                  }

                // sort triangles
                std::sort(points.begin(), points.end(), [&](id_type l, id_type r)
                {
                  double angleL = std::atan2(points_[l].y - points_[j].y,
                                             points_[l].x - points_[j].x);
                  double angleR = std::atan2(points_[r].y - points_[j].y,
                                             points_[r].x - points_[j].x);
                  return angleL < angleR;
                });
                std::rotate(points.begin(),
                            std::find(points.begin(), points.end(), first),
                            points.end());
                std::vector<size_t> neighbours_degrees(points.size());
                std::transform(points.begin(), points.end(),
                               neighbours_degrees.begin(),
                               [&](id_type id) {return triangles[id].size();});
                // triangulate
                triangulation = triangulate(points);
                triangulation.emplace_back(points.back(), points.front(), n);

                for (auto triangle: triangulation)
                  {
                    size_t size = add_triangle(triangle, triangles);
                    // update search dag
                    for (auto i: adjacent_triangles)
                      {
                        auto old_triangle = triangle_by_id(i);
                        auto new_triangle = triangle_by_id(size);
                        if (old_triangle.intersects(new_triangle))
                          {
                            // finites first
                            if (is_infinite(search_dag_.vertices[i]))
                              search_dag_.edges[size].push_back(i);
                            else
                              search_dag_.edges[size].push_front(i);
                          }
                      }
                  }

                // add new low degree points
                for (size_t i = 0; i < points.size(); ++i)
                  if (neighbours_degrees[i] >= DEGREE_THRESHOLD &&      // before
                      triangles[points[i]].size() < DEGREE_THRESHOLD && // after
                      points[i] != n)                                   // not root
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

        // last triangle
        assert(low_degree.size() == 3);
        triangulation.clear();
        if (find_triangle(low_degree[0],
                          low_degree[1],
                          low_degree[2]) == (int) search_dag_.vertices.size())
          triangulation.emplace_back(low_degree[0],
              low_degree[1],
              low_degree[2]);
        std::vector<id_type> adjacent_triangles;
        for (int i = 0; i < 3; i++)
          {
            if (find_triangle(low_degree[i],
                              low_degree[(i + 1) % 3],
                              n) == (int) search_dag_.vertices.size())
              triangulation.emplace_back(low_degree[i],
                                         low_degree[(i + 1) % 3],
                                         n);
            // edges
            adjacent_triangles.insert(adjacent_triangles.end(),
                                      triangles[low_degree[i]].begin(),
                                      triangles[low_degree[i]].end());
          }

        for (auto triangle: triangulation)
          {
            size_t size = add_triangle(triangle, triangles);
            // update search dag
            for (auto i: adjacent_triangles)
              {
                auto old_triangle = triangle_by_id(i);
                auto new_triangle = triangle_by_id(size);
                if (old_triangle.intersects(new_triangle))
                  {
                    // finites first
                    if (is_infinite(search_dag_.vertices[i]))
                      search_dag_.edges[size].push_back(i);
                    else
                      search_dag_.edges[size].push_front(i);
                  }
              }
          }

        // edges from fake root
        search_dag_.edges[0].push_back(find_triangle(low_degree[0],
                                                     low_degree[1],
                                                     low_degree[2]));
        for (int i = 0; i < 3; i++)
          search_dag_.edges[0].push_back(find_triangle(low_degree[i],
                                                       low_degree[(i + 1) % 3],
                                                       n));
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

      triangle_type<homogeneous_point_type>
      kirkpatrick_refinement::triangle_by_id(id_type id) const
      {
        assert(id < triangles_num());
        auto & ids = search_dag_.vertices[id];
        return triangle_type<homogeneous_point_type>(points_[ids.a],
            points_[ids.b],
            points_[ids.c]);
      }

      size_t kirkpatrick_refinement::triangles_num() const
      {
        return search_dag_.vertices.size();
      }

      kirkpatrick_refinement::id_type
      kirkpatrick_refinement::add_triangle(triangle_type<id_type> & t,
                                           std::vector<set_type> & triangles)
      {
        size_t size = search_dag_.vertices.size();
        //        if (t.c != points_.size() - 1)
        //          if (!is_left_turn(points_[t.a], points_[t.b], points_[t.c]))
        //            std::swap(t.b, t.c);
        search_dag_.vertices.push_back(t);
        search_dag_.edges.resize(search_dag_.vertices.size());
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
            assert(points_[id].omega != 0);
            if (id == id1 || id == id2 || id == id3)
              continue;
            if (triangle_type<homogeneous_point_type>(points_[id1],
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
        if (poly.size() < 3)
          return result;
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
        assert(size > 1);
        std::set<id_type> forbidden;
        forbidden.insert(points_.size() - 1);
        std::vector<id_type> result;
        id_type j = from.front();
        for (size_t i = 0; i < size; ++i)
          {
            if (from.size() == 3)
              break;
            assert(triangles[j].size() < DEGREE_THRESHOLD);
            assert(j != points_.size() - 1);
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
