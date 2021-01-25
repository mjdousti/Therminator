/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include <Eigen/SparseCore>
#include <Eigen/Core>

#include "device.hpp"
#include "general.hpp"
#include "rc_utils.hpp"
#include "utils.hpp"

/* model specific constants */
/* changed from 1/2 to 1/3 due to the difference from traditional Elmore Delay
 * scenario */
//#define C_FACTOR    0.33       /* fitting factor to match floworks (due to
// lumping)    */

class Model {
private:
  Device *device;

  Eigen::SparseMatrix<VALUE> g_matrix_;

  // A = C^-1 * G
  Eigen::SparseMatrix<VALUE> a_matrix_;


  // Inverted form of diagonal matrix C
  Eigen::DiagonalMatrix<VALUE, Eigen::Dynamic, Eigen::Dynamic> inv_c_;

  /**
   * This saves "device->getTemperature() * K1",
   * which comes from the lhs of the equation due to dropping
   * the term corresponds to the thermal coupling to the ambient.
   */
  Eigen::Matrix<VALUE, Eigen::Dynamic, 1> amb_vector_;
  Eigen::Matrix<VALUE, Eigen::Dynamic, 1> p_vector_;
  Eigen::Matrix<VALUE, Eigen::Dynamic, 1> t_vector_;

  bool p_vector_made_;
  bool g_matrix_made_;
  bool c_vector_made_;
  bool t_vector_made_;
  bool transient_;
  int elements_no_;
  unordered_map<string, int> powerMappingDeviceOrder;
  unordered_map<int, string> powerMappingTraceOrder;
  std::ifstream powerTraceFile;
  bool isComment(string s);
  /**
   * This designates the number of power consumers we
   * expect to find in the power trace file It is used to
   * check if the power trace has enough info
   */
  int pwr_consumers_cnt;
  string powerTraceFileAddr;

  void initPowerVector();
  void preparePVector();

public:
  void makePVector();
  void makeResistanceModel();
  // void makeResistanceModel2();
  void makeCapacitanceModel();

  /**
   * This function reads one line from the power trace file and place it
   * in the power vector
   *
   * @return Number of succesfully read power values from the last read row.
   */
  unsigned read_power();

  void solveSteadyState();
  void solveTransientState();
  void printSubComponentTemp();
  void printComponentTemp(string file_output);
  void printComponentTemp(string file_output, unsigned step_no);
  void printGMatrix(string file_output);
  void printAMatrix(string file_output);  
  void printInvCVector(string file_output);
  void printPVector(string file_output);
  void printTVector(string file_output);
  void printElementCount(string file_output);
  Model(Device *device, bool isTransient);
  virtual ~Model();
};
