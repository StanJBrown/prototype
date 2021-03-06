#ifndef PROTOTYPE_CALIB_APRILGRID_HPP
#define PROTOTYPE_CALIB_APRILGRID_HPP

// Order matters with the AprilTags lib. The detector has to be first.
#include <AprilTags/TagDetector.h>
#include <AprilTags/Tag36h11.h>

#include "prototype/core/core.hpp"
#include "prototype/vision/util.hpp"

namespace proto {

/**
 * AprilGrid Detector
 */
struct aprilgrid_detector_t {
  AprilTags::AprilGridDetector det;

  aprilgrid_detector_t();
  ~aprilgrid_detector_t();
};

/**
 * AprilGrid detection
 */
struct aprilgrid_t {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Grid properties
  bool configured = false;
  int tag_rows = 0;
  int tag_cols = 0;
  double tag_size = 0.0;
  double tag_spacing = 0.0;

  /// Detections
  bool detected = false;
  int nb_detections = 0;
  timestamp_t timestamp = 0;
  std::vector<int> ids;
  vec2s_t keypoints;

  /// Estimation
  bool estimated = false;
  vec3s_t points_CF;
  mat4_t T_CF = I(4);

  aprilgrid_t();
  aprilgrid_t(const timestamp_t &timestamp,
              const int tag_rows,
              const int tag_cols,
              const double tag_size,
              const double tag_spacing);
  ~aprilgrid_t();
};

typedef AprilTags::TagDetection apriltag_t;
typedef std::vector<aprilgrid_t> aprilgrids_t;

/** Add AprilTag measurement to AprilGrid. */
void aprilgrid_add(aprilgrid_t &grid,
                   const int id,
                   const std::vector<cv::Point2f> &keypoints);

/** Remove AprilTag measurement from AprilGrid based on id. */
void aprilgrid_remove(aprilgrid_t &grid, const int id);

/**
 * Get AprilTag measurements based on tag id.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_get(const aprilgrid_t &grid,
                  const int id,
                  vec2s_t &keypoints);

/**
 * Get AprilTag measurements based on tag id.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_get(const aprilgrid_t &grid,
                  const int id,
                  vec2s_t &keypoints,
                  vec3s_t &points_CF);

/** Set AprilGrid properties */
void aprilgrid_set_properties(aprilgrid_t &grid,
                              const int tag_rows,
                              const int tag_cols,
                              const double tag_size,
                              const double tag_spacing);

/**
 * Get the tag's grid index using the tag id
 * Note: The origin of the grid is bottom left corner rather than top left.
 * @returns 0 or -1 for success or failure
 */
int aprilgrid_grid_index(const aprilgrid_t &grid, const int id, int &i, int &j);

/**
 * Calculate object point.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_object_point(const aprilgrid_t &grid,
                           const int tag_id,
                           const int corner_id,
                           vec3_t &object_point);

/**
 * Calculate object points.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_object_points(const aprilgrid_t &grid,
                            const int tag_id,
                            vec3s_t &object_points);

/**
 * Calculate relative position between AprilGrid and camera using solvepnp.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_calc_relative_pose(aprilgrid_t &grid,
                                 const mat3_t &cam_K,
                                 const vec4_t &cam_D);

/** Show detection. */
void aprilgrid_imshow(const aprilgrid_t &grid,
                      const std::string &title,
                      const cv::Mat &image);

/**
 * Save AprilGrid detection.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_save(const aprilgrid_t &grid, const std::string &save_path);

/**
 * Load AprilGrid detection.
 * @returns 0 or -1 for success or failure.
 */
int aprilgrid_load(aprilgrid_t &grid, const std::string &data_path);

/**
 * Configure AprilGrid detector.
 * @returns 0 or 1 for success or failure.
 */
int aprilgrid_configure(aprilgrid_t &grid, const std::string &config_file);

/** Filter tags detected */
void aprilgrid_filter_tags(const cv::Mat &image,
                           std::vector<AprilTags::TagDetection> &tags);

/**
 * Detect AprilGrid.
 * @returns number of AprilTags detected
 */
int aprilgrid_detect(aprilgrid_t &grid,
                     const aprilgrid_detector_t &detector,
                     const cv::Mat &image);

/**
 * Detect AprilGrid.
 * @returns number of AprilTags detected.
 */
int aprilgrid_detect(aprilgrid_t &grid,
                     const aprilgrid_detector_t &detector,
                     const cv::Mat &image,
                     const mat3_t &cam_K,
                     const vec4_t &cam_D);

/**
 * Find the intersection of two aprilgrids
 */
void aprilgrid_intersection(aprilgrid_t &grid0, aprilgrid_t &grid1);

/** Comparator to sort detected AprilTags by id */
static bool sort_apriltag_by_id(const AprilTags::TagDetection &a,
                                const AprilTags::TagDetection &b) {
  return (a.id < b.id);
}

/** aprilgrid_t to output stream */
std::ostream &operator<<(std::ostream &os, const aprilgrid_t &april_grid);

} // namespace proto
#endif // PROTOTYPE_CALIB_APRILGRID_HPP
