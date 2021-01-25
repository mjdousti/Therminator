/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */


#include "headers/utils.hpp"

// Eigen::IOFormat OctaveFmt(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ";\n", "", "", "[", "]");
Eigen::IOFormat OctaveFmt(Eigen::StreamPrecision, Eigen::DontAlignCols, " ", "\n", "", "", "", "");

VALUE utils::KtoC(VALUE temp) { return temp - 273.15; }

void utils::dumpMatrix(const Eigen::SparseMatrix<VALUE>& matrix, const string& file_output) {
  std::ofstream temp_out(file_output, std::ofstream::out);
  temp_out << Eigen::MatrixXf(matrix).format(OctaveFmt) << std::endl;
  temp_out.close();
}

void utils::dumpVector(const Eigen::Matrix<VALUE, Eigen::Dynamic, 1>& vec, const string& file_output) {
  std::ofstream temp_out(file_output, std::ofstream::out);
  temp_out << vec.format(OctaveFmt) << std::endl;
  temp_out.close();
}

bool utils::fileExists(const std::string &name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  }
  return false;
}

VALUE utils::calcElapsedTime(
    const std::chrono::high_resolution_clock::time_point &start,
    const std::chrono::high_resolution_clock::time_point &end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
             .count() /
         1000.0;
}
