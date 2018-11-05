#include "prototype/vision/util.hpp"

namespace prototype {

cv::Mat roi(const cv::Mat &image,
            const int width,
            const int height,
            const double cx,
            const double cy) {
  const double x = cx - width / 2.0;
  const double y = cy - height / 2.0;
  cv::Rect roi(x, y, width, height);
  return image(roi);
}

bool is_rot_mat(const cv::Mat &R) {
  const cv::Mat Rt;
  cv::transpose(R, Rt);
  const cv::Mat I = cv::Mat::eye(3, 3, (Rt * R).type());
  return cv::norm(I, Rt * R) < 1e-6;
}

cv::Vec3f rot2euler(const cv::Mat &R) {
  assert(is_rot_mat(R));
  const double R00 = R.at<double>(0, 0);
  const double R10 = R.at<double>(1, 0);
  const float sy = sqrt(R00 * R00 + R10 * R10);
  bool singular = sy < 1e-6;

  float x;
  float y;
  float z;
  if (!singular) {
    x = atan2(R.at<double>(2, 1), R.at<double>(2, 2));
    y = atan2(-R.at<double>(2, 0), sy);
    z = atan2(R.at<double>(1, 0), R.at<double>(0, 0));

  } else {
    x = atan2(-R.at<double>(1, 2), R.at<double>(1, 1));
    y = atan2(-R.at<double>(2, 0), sy);
    z = 0;
  }

  return cv::Vec3f(x, y, z);
}

float rescale_points(std::vector<cv::Point2f> &pts1,
                     std::vector<cv::Point2f> &pts2) {
  // Calculate scaling factor
  float scaling_factor = 0.0f;
  for (size_t i = 0; i < pts1.size(); i++) {
    scaling_factor += sqrt(pts1[i].dot(pts1[i]));
    scaling_factor += sqrt(pts2[i].dot(pts2[i]));
  }
  scaling_factor = (pts1.size() + pts2.size()) / scaling_factor * sqrt(2.0f);

  // Rescale points
  for (size_t i = 0; i < pts1.size(); i++) {
    pts1[i] *= scaling_factor;
    pts2[i] *= scaling_factor;
  }

  return scaling_factor;
}

double reprojection_error(const std::vector<vec2_t> &measured,
                          const std::vector<vec2_t> &projected) {
  assert(measured.size() == projected.size());

  double sse = 0.0;
  const size_t nb_keypoints = measured.size();
  for (size_t i = 0; i < nb_keypoints; i++) {
    sse += (measured[i] - projected[i]).norm();
  }
  const double rmse = sqrt(sse / nb_keypoints);

  return rmse;
}

double reprojection_error(const std::vector<cv::Point2f> &measured,
                          const std::vector<cv::Point2f> &projected) {
  assert(measured.size() == projected.size());

  double sse = 0.0;
  const size_t nb_keypoints = measured.size();
  for (size_t i = 0; i < nb_keypoints; i++) {
    sse += cv::norm(measured[i] - projected[i]);
  }
  const double rmse = sqrt(sse / nb_keypoints);

  return rmse;
}

// void two_point_ransac(const std::vector<cv::Point2f> &pts1,
//                       const std::vector<cv::Point2f> &pts2,
//                       const cv::Matx33f &R_p_c,
//                       CameraProperty &cam,
//                       const double &inlier_error,
//                       const double &success_probability,
//                       std::vector<int> &inlier_markers) {
//
//   // Check the size of input point size.
//   if (pts1.size() != pts2.size()) {
//     LOG_ERROR("Sets of different size (%lu and %lu) are used...",
//               pts1.size(),
//               pts2.size());
//   }
//
//   const double cam_fx = cam.intrinsics[0];
//   const double cam_fy = cam.intrinsics[1];
//   double norm_pixel_unit = 2.0 / (cam_fx + cam_fy);
//   const int iter_num =
//       static_cast<int>(ceil(log(1 - success_probability) / log(1 - 0.7 *
//       0.7)));
//
//   // Initially, mark all points as inliers.
//   inlier_markers.clear();
//   inlier_markers.resize(pts1.size(), 1);
//
//   // Undistort all the points.
//   std::vector<cv::Point2f> pts1_ud = cam.undistortPoints(pts1);
//   std::vector<cv::Point2f> pts2_ud = cam.undistortPoints(pts2);
//
//   // Compenstate the points in the previous image with
//   // the relative rotation.
//   for (auto &pt : pts1_ud) {
//     const cv::vec3_tf pt_h(pt.x, pt.y, 1.0f);
//     // const cv::vec3_tf pt_hc = dR * pt_h;
//     const cv::vec3_tf pt_hc = R_p_c * pt_h;
//     pt.x = pt_hc[0];
//     pt.y = pt_hc[1];
//   }
//
//   // Normalize the points to gain numerical stability.
//   float scaling_factor = rescale_points(pts1_ud, pts2_ud);
//   norm_pixel_unit *= scaling_factor;
//
//   // Compute the difference between previous and current points,
//   // which will be used frequently later.
//   std::vector<cv::Point2d> pts_diff(pts1_ud.size());
//   for (size_t i = 0; i < pts1_ud.size(); i++) {
//     pts_diff[i] = pts1_ud[i] - pts2_ud[i];
//   }
//
//   // Mark the point pairs with large difference directly. BTW, the mean
//   // distance of the rest of the point pairs are computed.
//   double mean_pt_distance = 0.0;
//   int raw_inlier_cntr = 0;
//   for (size_t i = 0; i < pts_diff.size(); i++) {
//     double distance = sqrt(pts_diff[i].dot(pts_diff[i]));
//     // 25 pixel distance is a pretty large tolerance for normal motion.
//     // However, to be used with aggressive motion, this tolerance should
//     // be increased significantly to match the usage.
//     if (distance > 50.0 * norm_pixel_unit) {
//       inlier_markers[i] = 0;
//     } else {
//       mean_pt_distance += distance;
//       raw_inlier_cntr++;
//     }
//   }
//   mean_pt_distance /= raw_inlier_cntr;
//
//   // If the current number of inliers is less than 3, just mark
//   // all input as outliers. This case can happen with fast
//   // rotation where very few features are tracked.
//   if (raw_inlier_cntr < 3) {
//     for (auto &marker : inlier_markers) {
//       marker = 0;
//     }
//     return;
//   }
//
//   // Before doing 2-point RANSAC, we have to check if the motion
//   // is degenerated, meaning that there is no translation between
//   // the frames, in which case, the model of the RANSAC does not
//   // work. If so, the distance between the matched points will
//   // be almost 0.
//   // if (mean_pt_distance < inlier_error*norm_pixel_unit) {
//   if (mean_pt_distance < norm_pixel_unit) {
//     LOG_ERROR("Degenerated motion...");
//     for (size_t i = 0; i < pts_diff.size(); i++) {
//       if (inlier_markers[i] == 0) {
//         continue;
//       } else if (sqrt(pts_diff[i].dot(pts_diff[i])) >
//                  inlier_error * norm_pixel_unit) {
//         inlier_markers[i] = 0;
//       }
//     }
//
//     return;
//   }
//
//   // In the case of general motion, the RANSAC model can be applied.
//   // The three column corresponds to tx, ty, and tz respectively.
//   matx_t coeff_t(pts_diff.size(), 3);
//   for (size_t i = 0; i < pts_diff.size(); ++i) {
//     coeff_t(i, 0) = pts_diff[i].y;
//     coeff_t(i, 1) = -pts_diff[i].x;
//     coeff_t(i, 2) = pts1_ud[i].x * pts2_ud[i].y - pts1_ud[i].y *
//     pts2_ud[i].x;
//   }
//
//   std::vector<int> raw_inlier_idx;
//   for (size_t i = 0; i < inlier_markers.size(); ++i) {
//     if (inlier_markers[i] != 0)
//       raw_inlier_idx.push_back(i);
//   }
//
//   std::vector<int> best_inlier_set;
//   double best_error = 1e10;
//   std::random_device rd;  // Used to obtain a seed for the random number
//   engine std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded
//   with rd() std::uniform_int_distribution<> randint1(0, raw_inlier_idx.size()
//   - 1); std::uniform_int_distribution<> randint2(1, raw_inlier_idx.size() -
//   1);
//
//   for (int iter_idx = 0; iter_idx < iter_num; ++iter_idx) {
//     // Randomly select two point pairs.
//     // Although this is a weird way of selecting two pairs, but it
//     // is able to efficiently avoid selecting repetitive pairs.
//     int select_idx1 = randint1(gen);
//     int select_idx_diff = randint2(gen);
//     int select_idx2 =
//         select_idx1 + select_idx_diff < (int) raw_inlier_idx.size()
//             ? select_idx1 + select_idx_diff
//             : select_idx1 + select_idx_diff - raw_inlier_idx.size();
//     int pair_idx1 = raw_inlier_idx[select_idx1];
//     int pair_idx2 = raw_inlier_idx[select_idx2];
//
//     // Construct the model;
//     vec2_t coeff_tx(coeff_t(pair_idx1, 0), coeff_t(pair_idx2, 0));
//     vec2_t coeff_ty(coeff_t(pair_idx1, 1), coeff_t(pair_idx2, 1));
//     vec2_t coeff_tz(coeff_t(pair_idx1, 2), coeff_t(pair_idx2, 2));
//     std::vector<double> coeff_l1_norm(3);
//     coeff_l1_norm[0] = coeff_tx.lpNorm<1>();
//     coeff_l1_norm[1] = coeff_ty.lpNorm<1>();
//     coeff_l1_norm[2] = coeff_tz.lpNorm<1>();
//     int base_indicator =
//         min_element(coeff_l1_norm.begin(), coeff_l1_norm.end()) -
//         coeff_l1_norm.begin();
//
//     vec3_t model(0.0, 0.0, 0.0);
//     if (base_indicator == 0) {
//       mat2_t A;
//       A << coeff_ty, coeff_tz;
//       vec2_t solution = A.inverse() * (-coeff_tx);
//       model(0) = 1.0;
//       model(1) = solution(0);
//       model(2) = solution(1);
//     } else if (base_indicator == 1) {
//       mat2_t A;
//       A << coeff_tx, coeff_tz;
//       vec2_t solution = A.inverse() * (-coeff_ty);
//       model(0) = solution(0);
//       model(1) = 1.0;
//       model(2) = solution(1);
//     } else {
//       mat2_t A;
//       A << coeff_tx, coeff_ty;
//       vec2_t solution = A.inverse() * (-coeff_tz);
//       model(0) = solution(0);
//       model(1) = solution(1);
//       model(2) = 1.0;
//     }
//
//     // Find all the inliers among point pairs.
//     vecx_t error = coeff_t * model;
//     std::vector<int> inlier_set;
//     for (int i = 0; i < error.rows(); i++) {
//       if (inlier_markers[i] == 0) {
//         continue;
//       }
//       if (std::abs(error(i)) < inlier_error * norm_pixel_unit)
//         inlier_set.push_back(i);
//     }
//
//     // If the number of inliers is small, the current
//     // model is probably wrong.
//     if (inlier_set.size() < 0.2 * pts1_ud.size()) {
//       continue;
//     }
//
//     // Refit the model using all of the possible inliers.
//     vecx_t coeff_tx_better(inlier_set.size());
//     vecx_t coeff_ty_better(inlier_set.size());
//     vecx_t coeff_tz_better(inlier_set.size());
//     for (size_t i = 0; i < inlier_set.size(); ++i) {
//       coeff_tx_better(i) = coeff_t(inlier_set[i], 0);
//       coeff_ty_better(i) = coeff_t(inlier_set[i], 1);
//       coeff_tz_better(i) = coeff_t(inlier_set[i], 2);
//     }
//
//     vec3_t model_better(0.0, 0.0, 0.0);
//     if (base_indicator == 0) {
//       matx_t A(inlier_set.size(), 2);
//       A << coeff_ty_better, coeff_tz_better;
//       vec2_t solution =
//           (A.transpose() * A).inverse() * A.transpose() * (-coeff_tx_better);
//       model_better(0) = 1.0;
//       model_better(1) = solution(0);
//       model_better(2) = solution(1);
//     } else if (base_indicator == 1) {
//       matx_t A(inlier_set.size(), 2);
//       A << coeff_tx_better, coeff_tz_better;
//       vec2_t solution =
//           (A.transpose() * A).inverse() * A.transpose() * (-coeff_ty_better);
//       model_better(0) = solution(0);
//       model_better(1) = 1.0;
//       model_better(2) = solution(1);
//     } else {
//       matx_t A(inlier_set.size(), 2);
//       A << coeff_tx_better, coeff_ty_better;
//       vec2_t solution =
//           (A.transpose() * A).inverse() * A.transpose() * (-coeff_tz_better);
//       model_better(0) = solution(0);
//       model_better(1) = solution(1);
//       model_better(2) = 1.0;
//     }
//
//     // Compute the error and upate the best model if possible.
//     vecx_t new_error = coeff_t * model_better;
//
//     double this_error = 0.0;
//     for (const auto &inlier_idx : inlier_set) {
//       this_error += std::abs(new_error(inlier_idx));
//     }
//     this_error /= inlier_set.size();
//
//     if (inlier_set.size() > best_inlier_set.size()) {
//       best_error = this_error;
//       best_inlier_set = inlier_set;
//     }
//   }
//
//   // Fill in the markers
//   inlier_markers.clear();
//   inlier_markers.resize(pts1.size(), 0);
//   for (const auto &inlier_idx : best_inlier_set) {
//     inlier_markers[inlier_idx] = 1;
//   }
// }

matx_t feature_mask(const int image_width,
                    const int image_height,
                    const std::vector<cv::Point2f> points,
                    const int patch_width) {
  matx_t mask = ones(image_height, image_width);

  // Create a mask around each point
  for (const auto &p : points) {
    // Skip if pixel is out of image bounds
    const double px = static_cast<int>(p.x);
    const double py = static_cast<int>(p.y);
    if (px >= image_width || px <= 0) {
      continue;
    } else if (py >= image_height || py <= 0) {
      continue;
    }

    // Calculate patch top left corner, patch width and height
    vec2_t top_left{px - patch_width, py - patch_width};
    vec2_t top_right{px + patch_width, py - patch_width};
    vec2_t btm_left{px - patch_width, py + patch_width};
    vec2_t btm_right{px + patch_width, py + patch_width};
    std::vector<vec2_t *> corners{&top_left, &top_right, &btm_left, &btm_right};
    for (auto corner : corners) {
      // Check corner in x-axis
      if ((*corner)(0) < 0) {
        (*corner)(0) = 0;
      } else if ((*corner)(0) > image_width) {
        (*corner)(0) = image_width;
      }

      // Check corner in y-axis
      if ((*corner)(1) < 0) {
        (*corner)(1) = 0;
      } else if ((*corner)(1) > image_height) {
        (*corner)(1) = image_height;
      }
    }

    // Create mask around pixel
    const int row = top_left(1);
    const int col = top_left(0);
    int width = top_right(0) - top_left(0) + 1;
    int height = btm_left(1) - top_left(1) + 1;
    width = (col + width) > image_width ? width - 1 : width;
    height = (row + height) > image_height ? height - 1 : height;

    // std::cout << "---" << std::endl;
    // std::cout << image_width << std::endl;
    // std::cout << image_height << std::endl;
    // std::cout << row << std::endl;
    // std::cout << col << std::endl;
    // std::cout << width << std::endl;
    // std::cout << height << std::endl;
    // std::cout << "---" << std::endl;

    mask.block(row, col, height, width) = zeros(height, width);
  }

  return mask;
}

matx_t feature_mask(const int image_width,
                    const int image_height,
                    const std::vector<cv::KeyPoint> keypoints,
                    const int patch_width) {
  std::vector<cv::Point2f> points;
  for (const auto &kp : keypoints) {
    points.emplace_back(kp.pt);
  }

  return feature_mask(image_width, image_height, points, patch_width);
}

cv::Mat feature_mask_opencv(const int image_width,
                            const int image_height,
                            const std::vector<cv::Point2f> points,
                            const int patch_width) {
  auto mask = feature_mask(image_width, image_height, points, patch_width);

  cv::Mat mask_cv;
  cv::eigen2cv(mask, mask_cv);
  mask_cv.convertTo(mask_cv, CV_8UC1);

  return mask_cv;
}

cv::Mat feature_mask_opencv(const int image_width,
                            const int image_height,
                            const std::vector<cv::KeyPoint> keypoints,
                            const int patch_width) {
  auto mask = feature_mask(image_width, image_height, keypoints, patch_width);

  cv::Mat mask_cv;
  cv::eigen2cv(mask, mask_cv);
  mask_cv.convertTo(mask_cv, CV_8UC1);

  return mask_cv;
}

cv::Mat radtan_undistort_image(const mat3_t &K,
                               const vecx_t &D,
                               const cv::Mat &image) {
  cv::Mat image_ud;
  cv::Mat K_ud = convert(K).clone();
  cv::undistort(image, image_ud, convert(K), convert(D), K_ud);
  return image_ud;
}

cv::Mat equi_undistort_image(const mat3_t &K,
                             const vecx_t &D,
                             const cv::Mat &image,
                             cv::Mat &Knew,
                             const double balance) {
  // Estimate new camera matrix first
  const cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
  cv::fisheye::estimateNewCameraMatrixForUndistortRectify(convert(K),
                                                          convert(D),
                                                          image.size(),
                                                          R,
                                                          Knew,
                                                          balance);

  // Undistort image
  cv::Mat image_ud;
  cv::fisheye::undistortImage(image, image_ud, convert(K), convert(D), Knew);

  return image_ud;
}

} //  namespace prototype
