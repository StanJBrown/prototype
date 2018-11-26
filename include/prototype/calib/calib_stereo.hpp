#ifndef PROTOTYPE_CALIB_CALIB_STEREO_HPP
#define PROTOTYPE_CALIB_CALIB_STEREO_HPP

#include <ceres/ceres.h>

#include "prototype/core/core.hpp"
#include "prototype/calib/calib_data.hpp"

namespace prototype {

/**
 * Stereo camera calibration residual
 */
struct stereo_residual_t {
  double z_C0_[2] = {0.0, 0.0};      ///< Measurement from cam0
  double z_C1_[2] = {0.0, 0.0};      ///< Measurement from cam1
  double p_F_[3] = {0.0, 0.0, 0.0};  ///< Object point

  stereo_residual_t(const vec2_t &z_C0,
                    const vec2_t &z_C1,
                    const vec3_t &p_F);
  ~stereo_residual_t();

  /**
   * Calculate residual
   */
  template <typename T>
  bool operator()(const T *const cam0_intrinsics,
                  const T *const cam0_distortion,
                  const T *const cam1_intrinsics,
                  const T *const cam1_distortion,
                  const T *const q_C0C1,
                  const T *const t_C0C1,
                  const T *const q_C0F,
                  const T *const t_C0F,
                  T *residual) const;
};

} //  namespace prototype
#include "calib_stereo_impl.hpp"
#endif // PROTOTYPE_CALIB_CALIB_STEREO_HPP