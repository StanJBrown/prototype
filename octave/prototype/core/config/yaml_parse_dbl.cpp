#include "yaml_config.hpp"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  std::string yaml_path;
  std::string yaml_key;
  parse_args(nlhs, plhs, nrhs, prhs, yaml_path, yaml_key);

  try {
    config_t config{yaml_path};
    double value = 0;
    parse(config, yaml_key, value);

    plhs[0] = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
    double* data = (double*) mxGetData(plhs[0]);
    data[0] = value;

  } catch (const YAML::Exception& e) {
    mexErrMsgIdAndTxt("prototype:error", e.msg.c_str());
  }

  return;
}
