#include "turn.h"
#include "determinant.h"

int64_t turn(point_type const & p,
             point_type const & q,
             point_type const & r)
{
  return det3(p.x, p.y, 1,
              q.x, q.y, 1,
              r.x, r.y, 1);
}

bool is_left_turn(point_type const & p,
                  point_type const & q,
                  point_type const & r)
{
  return turn(p, q, r) > 0;
}
