#include <prototype/prototype.hpp>

using namespace proto;

struct calib_config_t {
  std::string target_file;
  std::string cam0_image_path;
  std::string cam1_image_path;
  std::string cam0_preprocess_path;
  std::string cam1_preprocess_path;

  vec2_t cam0_resolution{0.0, 0.0};
  double cam0_lens_hfov = 0.0;
  double cam0_lens_vfov = 0.0;
  std::string cam0_camera_model;
  std::string cam0_distortion_model;

  vec2_t cam1_resolution{0.0, 0.0};
  double cam1_lens_hfov = 0.0;
  double cam1_lens_vfov = 0.0;
  std::string cam1_camera_model;
  std::string cam1_distortion_model;
};

void print_usage() {
  const std::string usage = R"EOF(
Usage: calib_stereo <calib_config.yaml>

The `calib_config.yaml` file is expected to have the following format:

  calib:
    target_file: "aprilgrid_6x6.yaml"
    cam0_image_path: "/data/cam0"
    cam1_image_path: "/data/cam1"
    cam0_preprocess_path: "/tmp/calib/stereo/cam0"
    cam1_preprocess_path: "/tmp/calib/stereo/cam1"

  cam0:
    resolution: [752, 480]
    lens_hfov: 98.0
    lens_vfov: 73.0
    camera_model: "pinhole"
    distortion_model: "radtan"

  cam1:
    resolution: [752, 480]
    lens_hfov: 98.0
    lens_vfov: 73.0
    camera_model: "pinhole"
    distortion_model: "radtan"
)EOF";

  std::cout << usage << std::endl;
}

calib_config_t parse_config(const std::string &config_file) {
  config_t config{config_file};
  calib_config_t calib_conf;

  parse(config, "calib.target_file", calib_conf.target_file);
  parse(config, "calib.cam0_image_path", calib_conf.cam0_image_path);
  parse(config, "calib.cam1_image_path", calib_conf.cam1_image_path);
  parse(config, "calib.cam0_preprocess_path", calib_conf.cam0_preprocess_path);
  parse(config, "calib.cam1_preprocess_path", calib_conf.cam1_preprocess_path);

  parse(config, "cam0.resolution", calib_conf.cam0_resolution);
  parse(config, "cam0.lens_hfov", calib_conf.cam0_lens_hfov);
  parse(config, "cam0.lens_vfov", calib_conf.cam0_lens_vfov);
  parse(config, "cam0.camera_model", calib_conf.cam0_camera_model);
  parse(config, "cam0.distortion_model", calib_conf.cam0_distortion_model);

  parse(config, "cam1.resolution", calib_conf.cam1_resolution);
  parse(config, "cam1.lens_hfov", calib_conf.cam1_lens_hfov);
  parse(config, "cam1.lens_vfov", calib_conf.cam1_lens_vfov);
  parse(config, "cam1.camera_model", calib_conf.cam1_camera_model);
  parse(config, "cam1.distortion_model", calib_conf.cam1_distortion_model);

  return calib_conf;
}

int save_results(const std::string &save_path,
                 const vec2_t &cam0_resolution,
                 const vec2_t &cam1_resolution,
                 const pinhole_t &cam0_pinhole,
                 const radtan4_t &cam0_radtan,
                 const pinhole_t &cam1_pinhole,
                 const radtan4_t &cam1_radtan,
                 const mat4_t &T_C0C1) {
  std::ofstream outfile(save_path);
  const std::string indent = "  ";

  // Check if file is ok
  if (outfile.good() != true) {
    return -1;
  }

  // Save cam0 results
  {
    const std::string resolution = vec2str(cam0_resolution);
    const std::string intrinsics = arr2str(*cam0_pinhole.data, 4);
    const std::string distortion = arr2str(*cam0_radtan.data, 4);
    outfile << "cam0:" << std::endl;
    outfile << indent << "camera_model: \"pinhole\"" << std::endl;
    outfile << indent << "distortion_model: \"radtan\"" << std::endl;
    outfile << indent << "resolution: " << resolution << std::endl;
    outfile << indent << "intrinsics: " << intrinsics << std::endl;
    outfile << indent << "distortion: " << distortion << std::endl;
    outfile << std::endl;
  }

  // Save cam1 results
  {
    const std::string resolution = vec2str(cam1_resolution);
    const std::string intrinsics = arr2str(*cam1_pinhole.data, 4);
    const std::string distortion = arr2str(*cam1_radtan.data, 4);
    outfile << "cam1:" << std::endl;
    outfile << indent << "camera_model: \"pinhole\"" << std::endl;
    outfile << indent << "distortion_model: \"radtan\"" << std::endl;
    outfile << indent << "resolution: " << resolution << std::endl;
    outfile << indent << "intrinsics: " << intrinsics << std::endl;
    outfile << indent << "distortion: " << distortion << std::endl;
    outfile << std::endl;
  }

  // Save camera extrinsics
  outfile << "T_C0C1: " << std::endl;
  outfile << indent << "rows: 4" << std::endl;
  outfile << indent << "cols: 4" << std::endl;
  outfile << indent << "data: [" << std::endl;
  outfile << mat2str(T_C0C1, indent + indent) << std::endl;
  outfile << indent << "]" << std::endl;
  outfile << std::endl;

  // Finsh up
  outfile.close();

  return 0;
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  if (argc != 2) {
    print_usage();
    return -1;
  }
  const std::string calib_config_file{argv[1]};

  // Parse calib config file
  const std::string config_file{argv[1]};
  const calib_config_t config = parse_config(config_file);

  // Load calibration target
  calib_target_t calib_target;
  if (calib_target_load(calib_target, config.target_file) != 0) {
    LOG_ERROR("Failed to load calib target [%s]!", config.target_file.c_str());
    return -1;
  }

  // Preprocess calibration data
  int retval = preprocess_stereo_data(calib_target,
                                      config.cam0_image_path,
                                      config.cam1_image_path,
                                      config.cam0_resolution,
                                      config.cam1_resolution,
                                      config.cam0_lens_hfov,
                                      config.cam0_lens_vfov,
                                      config.cam1_lens_hfov,
                                      config.cam1_lens_vfov,
                                      config.cam0_preprocess_path,
                                      config.cam1_preprocess_path);
  if (retval != 0) {
    LOG_ERROR("Failed to preprocess calibration data!");
    return -1;
  }

  // Load stereo calibration data
  std::vector<aprilgrid_t> cam0_aprilgrids;
  std::vector<aprilgrid_t> cam1_aprilgrids;
  retval = load_stereo_calib_data(config.cam0_preprocess_path,
                                  config.cam1_preprocess_path,
                                  cam0_aprilgrids,
                                  cam1_aprilgrids);
  if (retval != 0) {
    LOG_ERROR("Failed to load calibration data!");
    return -1;
  }

  // Setup initial cam0 intrinsics and distortion
  // -- cam0
  const double cam0_img_w = config.cam0_resolution(0);
  const double cam0_img_h = config.cam0_resolution(1);
  const double cam0_lens_hfov = config.cam0_lens_hfov;
  const double cam0_lens_vfov = config.cam0_lens_vfov;;
  const double cam0_fx = pinhole_focal_length(cam0_img_w, cam0_lens_hfov);
  const double cam0_fy = pinhole_focal_length(cam0_img_h, cam0_lens_vfov);
  const double cam0_cx = cam0_img_w / 2.0;
  const double cam0_cy = cam0_img_h / 2.0;
  pinhole_t cam0_pinhole{cam0_fx, cam0_fy, cam0_cx, cam0_cy};
  radtan4_t cam0_radtan{0.01, 0.0001, 0.0001, 0.0001};
  // -- cam1
  const double cam1_img_w = config.cam1_resolution(0);
  const double cam1_img_h = config.cam1_resolution(1);
  const double cam1_lens_hfov = config.cam1_lens_hfov;
  const double cam1_lens_vfov = config.cam1_lens_vfov;;
  const double cam1_fx = pinhole_focal_length(cam1_img_w, cam1_lens_hfov);
  const double cam1_fy = pinhole_focal_length(cam1_img_h, cam1_lens_vfov);
  const double cam1_cx = cam1_img_w / 2.0;
  const double cam1_cy = cam1_img_h / 2.0;
  pinhole_t cam1_pinhole{cam1_fx, cam1_fy, cam1_cx, cam1_cy};
  radtan4_t cam1_radtan{0.01, 0.0001, 0.0001, 0.0001};

  // Calibrate stereo
  LOG_INFO("Calibrating stereo camera!");
  mat4_t T_C0C1 = I(4);
  mat4s_t poses;
  retval = calib_stereo_solve(cam0_aprilgrids, cam1_aprilgrids,
                              cam0_pinhole, cam0_radtan,
                              cam1_pinhole, cam1_radtan,
                              T_C0C1,
                              poses);
  if (retval != 0) {
    LOG_ERROR("Failed to calibrate stereo cameras!");
    return -1;
  }

  // Show results
  std::cout << "Optimization results:" << std::endl;
  // -- cam0
  std::cout << "cam0:" << std::endl;
  std::cout << cam0_pinhole << std::endl;
  std::cout << cam0_radtan << std::endl;
  // -- cam1
  std::cout << "cam1:" << std::endl;
  std::cout << cam1_pinhole << std::endl;
  std::cout << cam1_radtan << std::endl;
  // -- cam0-cam1 extrinsics
  std::cout << "T_C0C1:\n" << T_C0C1 << std::endl;
  std::cout << std::endl;

  // Save results
  const std::string save_path{"./calib_results.yaml"};
  LOG_INFO("Saving optimization results to [%s]", save_path.c_str());
  retval = save_results(save_path,
                        config.cam0_resolution,
                        config.cam1_resolution,
                        cam0_pinhole,
                        cam0_radtan,
                        cam1_pinhole,
                        cam1_radtan,
                        T_C0C1);
  if (retval != 0) {
    LOG_ERROR("Failed to save results to [%s]!", save_path.c_str());
    return -1;
  }

  return 0;
}
