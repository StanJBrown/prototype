#include "prototype/core/config.hpp"

namespace prototype {

config_t::config_t(const std::string &file_path_) :
  file_path{file_path_} {
  if (yaml_load_file(file_path_, root) == 0) {
    ok = true;
  }
}

int yaml_load_file(const std::string file_path,
                   YAML::Node &root) {
  // Pre-check
  if (file_exists(file_path) == false) {
    LOG_ERROR("File not found: %s", file_path.c_str());
    return -1;
  }

  // Load and parse file
  root = YAML::LoadFile(file_path);
  return 0;
}

int yaml_get_node(const config_t &config,
                  const std::string &key,
                  const bool optional,
                  YAML::Node &node) {
  ASSERT(config_.ok == true, "Config file is not loaded!");

  // Recurse down config key
  std::vector<YAML::Node> traversal;
  traversal.push_back(config.root);

  std::istringstream iss(key);
  std::string element;

  while (std::getline(iss, element, '.')) {
    traversal.push_back(traversal.back()[element]);
  }
  node = traversal.back();
  // Note:
  //
  //    yaml_node = yaml_node["some_level_deeper"];
  //
  // YAML::Node is mutable, by doing the above it destroys the parsed yaml
  // tree/graph, to avoid this problem we store the visited YAML::Node into
  // a std::vector and return the last visited YAML::Node

  // Check key
  if (node.IsDefined() == false && optional == false) {
    FATAL("Opps [%s] missing in yaml file [%s]!",
          key.c_str(),
          config.file_path.c_str());
    return -1;
  } else if (node.IsDefined() == false && optional == true) {
    return -1;
  }

  return 0;
}

void parse(const config_t &config, const std::string &key, vec2_t &vec, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  const size_t vector_size = yaml_check_vector<vec2_t>(node, key, optional);
  vec = vec2_t{node[0].as<double>(), node[1].as<double>()};
}

void parse(const config_t &config, const std::string &key, vec3_t &vec, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  const size_t vector_size = yaml_check_vector<vec3_t>(node, key, optional);
  vec = vec3_t{node[0].as<double>(), node[1].as<double>(), node[2].as<double>()};
}

void parse(const config_t &config, const std::string &key, vec4_t &vec, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  const size_t vector_size = yaml_check_vector<vec4_t>(node, key, optional);
  vec = vec4_t{node[0].as<double>(), node[1].as<double>(),
               node[2].as<double>(), node[3].as<double>()};
}

void parse(const config_t &config, const std::string &key, vecx_t &vec, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  const size_t vector_size = yaml_check_vector<vecx_t>(node, key, optional);
  vec = vecx_t::Zero(vector_size, 1);
  for (size_t i = 0; i < node.size(); i++) {
    vec(i) = node[i].as<double>();
  }
}

void parse(const config_t &config, const std::string &key, mat2_t &mat, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  yaml_check_matrix<mat2_t>(node, key, optional);
  mat(0, 0) = node["data"][0].as<double>();
  mat(0, 1) = node["data"][1].as<double>();
  mat(1, 0) = node["data"][2].as<double>();
  mat(1, 1) = node["data"][3].as<double>();
}

void parse(const config_t &config, const std::string &key, mat3_t &mat, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  yaml_check_matrix<mat3_t>(node, key, optional);
  // -- Col 1
  mat(0, 0) = node["data"][0].as<double>();
  mat(0, 1) = node["data"][1].as<double>();
  mat(0, 2) = node["data"][2].as<double>();
  // -- Col 2
  mat(1, 0) = node["data"][3].as<double>();
  mat(1, 1) = node["data"][4].as<double>();
  mat(1, 2) = node["data"][5].as<double>();
  // -- Col 3
  mat(2, 0) = node["data"][6].as<double>();
  mat(2, 1) = node["data"][7].as<double>();
  mat(2, 2) = node["data"][8].as<double>();
}

void parse(const config_t &config, const std::string &key, mat4_t &mat, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  yaml_check_matrix<mat4_t>(node, key, optional);
  size_t index = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mat(i, j) = node["data"][index].as<double>();
      index++;
    }
  }
}

void parse(const config_t &config, const std::string &key, matx_t &mat, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  size_t rows = 0;
  size_t cols = 0;
  yaml_check_matrix<matx_t>(node, key, optional, rows, cols);

  mat.resize(rows, cols);
  size_t index = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      mat(i, j) = node["data"][index].as<double>();
      index++;
    }
  }
}

void parse(const config_t &config, const std::string &key, cv::Mat &mat, const bool optional) {
  // Get node
  YAML::Node node;
  if (yaml_get_node(config, key, optional, node) != 0) {
    return;
  }

  // Parse
  size_t rows = 0;
  size_t cols = 0;
  yaml_check_matrix<cv::Mat>(node, key, optional, rows, cols);

  mat = cv::Mat(rows, cols, CV_64F);
  size_t index = 0;
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      mat.at<double>(i, j) = node["data"][index].as<double>();
      index++;
    }
  }
}

} //  namespace prototype
