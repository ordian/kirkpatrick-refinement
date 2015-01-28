#ifndef _TURN_H
#define _TURN_H

#include "determinant.h"
#include "geom/primitives/point.h"

using geom::structures::homogeneous_point_type;

int64_t turn(homogeneous_point_type const & p,
             homogeneous_point_type const & q,
             homogeneous_point_type const & r)
{
  return det3(p.x, p.y, p.omega,
              q.x, q.y, q.omega,
              r.x, r.y, r.omega);
}

bool is_left_turn(homogeneous_point_type const & p,
                  homogeneous_point_type const & q,
                  homogeneous_point_type const & r)
{
  return turn(p, q, r) > 0;
}

#endif // _TURN_H
