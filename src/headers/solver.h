/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

#pragma once

#include "general.h"
#include <vector>
#include <Eigen/Core>
#include <Eigen/Sparse>


typedef Eigen::Matrix<VALUE, Eigen::Dynamic, 1> state_type;
typedef Eigen::SparseMatrix<VALUE> matrix_type;

#if USE_GPU == 1

#include <cuda_runtime.h>
#include <cusolverDn.h>
#include <cusolverSp.h>
#include <cusparse.h>

#else
#define EIGEN_SUPERLU_SUPPORT
#define EIGEN_USE_MKL_ALL

// A quick and dirty fix to support MKL 11.2
// http://eigen.tuxfamily.org/bz/show_bug.cgi?id=874
#ifndef MKL_BLAS
#define MKL_BLAS MKL_DOMAIN_BLAS
#endif

#include <boost/numeric/odeint/external/eigen/eigen.hpp>

#include <Eigen/Dense>
#include <Eigen/IterativeLinearSolvers>
#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp>
// #include <boost/numeric/odeint/external/eigen/eigen_algebra.hpp>
// #include <boost/numeric/odeint/algebra/vector_space_algebra.hpp>

// #include <boost/numeric/odeint/algebra/algebra_dispatcher.hpp>
// #include <boost/numeric/odeint/algebra/vector_space_algebra.hpp>

#include <unsupported/Eigen/IterativeSolvers>

// using namespace Eigen;
// using namespace boost::numeric::odeint;

#endif

namespace solver {

#if USE_GPU == 1
void gpuSolveDenseSteady(const matrix_type& matrix, const state_type& p_vector,
                         state_type t_vector, bool use_cholesky = true);

void gpuSolveSparseSteady(const matrix_type& matrix, const state_type& p_vector,
                                  state_type& t_vector);

void gpuSolveTransient();
#else
void cpuSolveSteady(const matrix_type &matrix,
                    const state_type &p_vector, state_type &t_vector);
void cpuSolveTransient(
    const matrix_type &a_matrix,
    const Eigen::DiagonalMatrix<VALUE, Eigen::Dynamic, Eigen::Dynamic> &invC,
    const state_type &p_vector, state_type &t_vector, VALUE time_start,
    VALUE time_end);

#endif

void writeTemp(const state_type &t_vector_, const VALUE t);
}; // namespace solver
