#ifndef _TURN_H
#define _TURN_H

#include "geom/primitives/point.h"

using geom::structures::point_type;

int64_t turn(point_type const & p,
             point_type const & q,
             point_type const & r);

bool is_left_turn(point_type const & p,
                  point_type const & q,
                  point_type const & r);

#endif // _TURN_H
