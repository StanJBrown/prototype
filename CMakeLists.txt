CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(prototype)

# C++ COMPILER SETTINGS
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(BUILD_SHARED_LIBS ON)
SET(CMAKE_BUILD_TYPE RELEASE)
# SET(CMAKE_BUILD_TYPE DEBUG)

# DEPENDENCIES
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(Ceres REQUIRED)
FIND_PACKAGE(Eigen3 REQUIRED)
FIND_PACKAGE(SuiteSparse REQUIRED)
FIND_PACKAGE(OpenMP REQUIRED)
INCLUDE(cmake/ImportEigen3.cmake)
if (OPENMP_FOUND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# INCLUDES
INCLUDE_DIRECTORIES(include ${EIGEN3_INCLUDE_DIR} ${SUITESPARSE_INCLUDE_DIRS})

# LIBRARY
ADD_LIBRARY(
  ${PROJECT_NAME}
  # calib
  src/calib/aprilgrid.cpp
  src/calib/calib_data.cpp
  src/calib/calib_camera.cpp
  src/calib/calib_gimbal.cpp
  src/calib/calib_stereo.cpp
  src/calib/calib_vicon_marker.cpp
  src/calib/chessboard.cpp
  # control
  src/control/carrot_ctrl.cpp
  src/control/pid.cpp
  # core
  src/core/config.cpp
  src/core/data.cpp
  src/core/file.cpp
  src/core/gps.cpp
  src/core/jpl.cpp
  src/core/linalg.cpp
  src/core/math.cpp
  src/core/tf.cpp
  src/core/time.cpp
  # dataset
  src/dataset/euroc.cpp
  src/dataset/kitti.cpp
  src/dataset/timeline.cpp
  # driver
  src/driver/camera.cpp
  src/driver/i2c.cpp
  src/driver/mpu6050.cpp
  src/driver/pca9685.cpp
  src/driver/sbgc.cpp
  src/driver/uart.cpp
  # mav
  src/mav/att_ctrl.cpp
  src/mav/mission.cpp
  src/mav/pos_ctrl.cpp
  src/mav/wp_ctrl.cpp
  # model
  src/model/gimbal.cpp
  src/model/mav.cpp
  src/model/two_wheel.cpp
  # vision
  src/vision/camera/radtan.cpp
  src/vision/camera/equi.cpp
  src/vision/camera/pinhole.cpp
  src/vision/feature2d/draw.cpp
  src/vision/feature2d/grid_fast.cpp
  src/vision/feature2d/grid_good.cpp
  src/vision/util.cpp
)
SET(
  ${PROJECT_NAME}_DEPS
  ${OpenCV_LIBS}
  ${SUITESPARSE_LIBRARIES}
  yaml-cpp
  apriltags
  ceres
)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_DEPS})
INSTALL(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
INSTALL(
  DIRECTORY include/
  DESTINATION /usr/local/include
  FILES_MATCHING PATTERN "*.hpp"
)

# UNIT TESTS
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
FILE(COPY tests/test_data DESTINATION ${PROJECT_BINARY_DIR}/tests)
SET(
  UNITTESTS
  calib-aprilgrid_test
  calib-calib_data_test
  calib-calib_camera_test
  # calib-calib_gimbal_test
  calib-calib_stereo_test
  calib-chessboard_test
  control-carrot_ctrl_test
  control-pid_test
  core-config_test
  core-data_test
  core-file_test
  core-gps_test
  core-linalg_test
  core-math_test
  core-tf_test
  core-time_test
  dataset-euroc_test
  dataset-kitti_test
  driver-camera_test
  model-mav_test
  model-two_wheel_test
  mav-att_ctrl_test
  mav-mission_test
  mav-pos_ctrl_test
  mav-wp_ctrl_test
  vision-camera-camera_geometry_test
  vision-camera-equi_test
  vision-camera-pinhole_test
  vision-camera-radtan_test
  vision-feature2d-grid_fast_test
  vision-feature2d-grid_good_test
  vision-util_test
)
FOREACH(TEST ${UNITTESTS})
  STRING(REGEX REPLACE "-" "/" TEST_PATH ${TEST})
  ADD_EXECUTABLE(${TEST} tests/${TEST_PATH}.cpp)
  TARGET_LINK_LIBRARIES(${TEST} ${PROJECT_NAME} ${${PROJECT_NAME}_DEPS})
ENDFOREACH(TEST)

# APPS
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/apps)
FILE(COPY apps/config DESTINATION ${PROJECT_BINARY_DIR}/apps)
SET(
  APP_DEPS
  prototype
  ${OpenCV_LIBS}
  ${SUITESPARSE_LIBRARIES}
  yaml-cpp
  apriltags
  ceres
)
SET(
  APPS
  calib_camera
  calib_stereo
  calib_vicon_marker
  detect_aprilgrid
  validate_intrinsics
  validate_stereo
)
FOREACH(APP ${APPS})
  ADD_EXECUTABLE(${APP} apps/${APP}.cpp)
  TARGET_LINK_LIBRARIES(${APP} ${APP_DEPS})
ENDFOREACH(APP)
