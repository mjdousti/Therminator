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

#include "general.h"

// OpenMP header file
#include <omp.h>

#define NEGLIGIBLE_EPSILON 1e-5

namespace utils {
VALUE KtoC(VALUE temp);

void dumpMatrix(const Eigen::SparseMatrix<VALUE>& matrix, const string& file_output);
void dumpVector(const Eigen::Matrix<VALUE, Eigen::Dynamic, 1>& vec, const string& file_output);

bool fileExists(const std::string &name);

VALUE calcElapsedTime(
    const std::chrono::high_resolution_clock::time_point &start,
    const std::chrono::high_resolution_clock::time_point &end);

// Inline function for speed up.
inline bool eq(VALUE a, VALUE b) { return fabs(a - b) < NEGLIGIBLE_EPSILON; }
inline bool neq(VALUE a, VALUE b) { return !eq(a, b); }
inline bool less(VALUE a, VALUE b) { return (b - a) > NEGLIGIBLE_EPSILON; }
inline bool le(VALUE a, VALUE b) { return ((a < b) || eq(a, b)); }
inline bool ge(VALUE a, VALUE b) { return ((a > b) || eq(a, b)); }
inline bool greater(VALUE a, VALUE b) { return (a - b) > NEGLIGIBLE_EPSILON; }

}; // namespace utils
