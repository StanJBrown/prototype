#include <unistd.h>

#include "prototype/munit.hpp"
#include "prototype/core/tf.hpp"
#include "prototype/core/euler.hpp"
#include "prototype/core/time.hpp"

namespace prototype {

int test_tf_rot() {
  mat4_t T_WS = I(4);
  T_WS.block<3, 3>(0, 0) = 1.0 * ones(3);
  T_WS.block<3, 1>(0, 3) = 2.0 * ones(3, 1);

  MU_CHECK((1.0 * ones(3) - tf_rot(T_WS)).norm() < 1e-5);

  return 0;
}

int test_tf_trans() {
  mat4_t T_WS = I(4);
  T_WS.block<3, 3>(0, 0) = 1.0 * ones(3);
  T_WS.block<3, 1>(0, 3) = 2.0 * ones(3, 1);

  MU_CHECK((2.0 * ones(3, 1) - tf_trans(T_WS)).norm() < 1e-5);

  return 0;
}

void test_suite() {
  MU_ADD_TEST(test_tf_rot);
  MU_ADD_TEST(test_tf_trans);
}

} // namespace prototype

MU_RUN_TESTS(prototype::test_suite);