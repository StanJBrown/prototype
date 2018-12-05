#include "prototype/core/tf.hpp"

namespace prototype {

mat4_t tf(const mat3_t &C, const vec3_t &r) {
  mat4_t T = zeros(4, 4);
  T.block(0, 0, 3, 3) = C;
  T.block(0, 3, 3, 1) = r;
  T(3, 3) = 1.0;

  return T;
}

mat4_t tf(const quat_t &q, const vec3_t &r) {
  return tf(q.toRotationMatrix(), r);
}

} // namespace prototype