#ifndef PROTOTYPE_CALIB_CALIB_CAMERA_HPP
#define PROTOTYPE_CALIB_CALIB_CAMERA_HPP

#include <iostream>
#include <string>
#include <memory>

#include <ceres/ceres.h>

#include "prototype/core/core.hpp"
#include "prototype/calib/ceres.hpp"
#include "prototype/calib/calib_data.hpp"
#include "prototype/vision/camera/radtan.hpp"

namespace proto {

/**
 * Pinhole Radial-tangential calibration residual
 */
struct pinhole_radtan4_residual_t {
  double z_[2] = {0.0, 0.0};        ///< Measurement
  double p_F_[3] = {0.0, 0.0, 0.0}; ///< Object point

  pinhole_radtan4_residual_t(const vec2_t &z, const vec3_t &p_F);
  ~pinhole_radtan4_residual_t();

  /**
   * Calculate residual (auto diff version)
   */
  template <typename T>
  bool operator()(const T *const intrinsics_,
                  const T *const distortion_,
                  const T *const q_CF_,
                  const T *const t_CF_,
                  T *residual_) const;
};

struct intrinsics_residual_t
  : public ceres::SizedCostFunction<2,  /* number of residuals */
                                    4,  /* size of intrinsics parameter */
                                    4,  /* size of distortion parameter */
                                    4,  /* size of q_CF parameter */
                                    3> {  /* size of t_CF parameter */
  double z_[2] = {0.0, 0.0};        ///< Measurement
  double p_F_[3] = {0.0, 0.0, 0.0}; ///< Object point

  intrinsics_residual_t(const vec2_t &z, const vec3_t &p_F);
  ~intrinsics_residual_t();

  /**
   * Calculate residual (analytical version)
   */
  virtual bool Evaluate(double const* const* parameters,
                        double* residuals,
                        double** jacobians) const;
};

/**
 * Calibrate camera intrinsics and relative pose between camera and fiducial
 * calibration target.
 *
 * @returns 0 or -1 for success or failure
 */
int calib_camera_solve(const std::vector<aprilgrid_t> &aprilgrids,
                       pinhole_t &pinhole,
                       radtan4_t &radtan,
                       mat4s_t &poses);

/**
 * Perform stats analysis on calibration after performing intrinsics
 * calibration.
 *
 * @returns 0 or -1 for success or failure
 */
template <typename RESIDUAL>
int calib_camera_stats(const std::vector<aprilgrid_t> &aprilgrids,
                       const double *intrinsics,
                       const double *distortion,
                       const mat4s_t &poses,
                       const std::string &output_path);

} //  namespace proto
#include "calib_camera_impl.hpp"
#endif // PROTOTYPE_CALIB_CALIB_CAMERA_HPP
