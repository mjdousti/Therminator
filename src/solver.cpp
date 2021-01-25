/**
 *
 * Copyright (C) 2021 Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi,
 * and Massoud Pedram. All rights reserved.
 *
 * Please refer to the LICENSE file for terms of use.
 *
 */

// These three header files override their Boost counter part. They should be
// imported first.

#if USE_GPU == 0

#include "headers/implicit_euler.hpp"
#include "headers/rosenbrock4.hpp"
#include "headers/rosenbrock4_controller.hpp"

#endif

#include "headers/solver.hpp"
#include "headers/utils.hpp"
#include <omp.h>

#if USE_GPU == 1

#include <Eigen/IterativeLinearSolvers>
#include <helper_cuda.h>

void solver::gpuSolveDenseSteady(const matrix_type &matrix,
                                 const state_type &p_vector,
                                 state_type t_vector, bool use_cholesky) {
  cusolverDnHandle_t cusolverH = NULL;
  cudaStream_t stream = NULL;

  const int n = p_vector.size();
  int info = 0; /* host copy of error info */

  VALUE *d_matrix = NULL;   /* device copy of A */
  VALUE *d_p_vector = NULL; /* device copy of B */
  int *d_Ipiv = NULL;       /* pivoting sequence */
  int *d_info = NULL;       /* error info */
  int lwork = 0;            /* size of workspace */
  VALUE *d_work = NULL;     /* device workspace for getrf */

  // step 1: create cusolver handle, bind a stream
  checkCudaErrors(cusolverDnCreate(&cusolverH));
  checkCudaErrors(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
  checkCudaErrors(cusolverDnSetStream(cusolverH, stream));

  // step 2: copy matrices to device
  checkCudaErrors(cudaMalloc((void **)&d_matrix, sizeof(VALUE) * n * n));
  checkCudaErrors(cudaMalloc((void **)&d_p_vector, sizeof(VALUE) * n));
  checkCudaErrors(cudaMalloc((void **)&d_Ipiv, sizeof(int) * n));
  checkCudaErrors(cudaMalloc((void **)&d_info, sizeof(int)));
  {
    Eigen::MatrixXf dense_representation = Eigen::MatrixXf(matrix);
    checkCudaErrors(cudaMemcpy(d_matrix, dense_representation.data(),
                               sizeof(VALUE) * n * n, cudaMemcpyHostToDevice));
  }
  checkCudaErrors(cudaMemcpy(d_p_vector, p_vector.data(), sizeof(VALUE) * n,
                             cudaMemcpyHostToDevice));

  // step 3: query working space of getrf
  if (use_cholesky) {
    checkCudaErrors(cusolverDnSpotrf_bufferSize(
        cusolverH, CUBLAS_FILL_MODE_LOWER, n, d_matrix, n, &lwork));
  } else {
    checkCudaErrors(
        cusolverDnSgetrf_bufferSize(cusolverH, n, n, d_matrix, n, &lwork));
  }
  checkCudaErrors(cudaMalloc((void **)&d_work, sizeof(VALUE) * lwork));

  // step 4: LUP factorization
  if (use_cholesky) {
    checkCudaErrors(cusolverDnSpotrf(cusolverH, CUBLAS_FILL_MODE_LOWER, n,
                                     d_matrix, n, d_work, lwork, d_info));
  } else {
    checkCudaErrors(
        cusolverDnSgetrf(cusolverH, n, n, d_matrix, n, d_work, d_Ipiv, d_info));
  }

  checkCudaErrors(
      cudaMemcpy(&info, d_info, sizeof(int), cudaMemcpyDeviceToHost));
  ASSERT(info >= 0, -info << "-th parameter is wrong.");

  // step 5: solve A*X = B
  if (use_cholesky) {
    checkCudaErrors(cusolverDnSpotrs(cusolverH, CUBLAS_FILL_MODE_LOWER, n,
                                     1, /* nrhs */
                                     d_matrix, n, d_p_vector, n, d_info));
  } else {
    checkCudaErrors(cusolverDnSgetrs(cusolverH, CUBLAS_OP_N, n, 1, /* nrhs */
                                     d_matrix, n, d_Ipiv, d_p_vector, n,
                                     d_info));
  }

  checkCudaErrors(cudaDeviceSynchronize());
  checkCudaErrors(cudaMemcpy(t_vector.data(), d_p_vector, sizeof(VALUE) * n,
                             cudaMemcpyDeviceToHost));

  /* free resources */
  checkCudaErrors(cudaFree(d_matrix));
  checkCudaErrors(cudaFree(d_p_vector));
  checkCudaErrors(cudaFree(d_Ipiv));
  checkCudaErrors(cudaFree(d_info));
  checkCudaErrors(cudaFree(d_work));
  checkCudaErrors(cusolverDnDestroy(cusolverH));
  checkCudaErrors(cudaStreamDestroy(stream));

  checkCudaErrors(cudaDeviceReset());
}

/**
 * Taken from
 * https://github.com/phrb/intro-cuda/blob/master/src/cuda-samples/7_CUDALibraries/cuSolverSp_LinearSolver/cuSolverSp_LinearSolver.cpp
 */
void solver::gpuSolveSparseSteady(const matrix_type &matrix,
                                  const state_type &p_vector,
                                  state_type &t_vector) {
  cusolverSpHandle_t handle = NULL;
  cusparseHandle_t cusparseHandle = NULL; // used in residual evaluation
  cudaStream_t stream = NULL;
  cusparseMatDescr_t descrA = NULL;

  const int n = p_vector.size();
  const int nnzA = matrix.nonZeros();          // number of nonzeros of A
  const int baseA = matrix.outerIndexPtr()[0]; // base index in CSR format

  int *d_csrRowPtrA = NULL;
  int *d_csrColIndA = NULL;
  VALUE *d_csrValA = NULL;
  VALUE *d_t_vector = NULL; // x = A \ b
  VALUE *d_p_vector = NULL; // a copy of h_b

  const VALUE tol = 1.e-12;
  const int reorder = 0;     // no reordering

  int singularity = 0; // -1 if A is invertible under tol.

  VALUE start, stop;
  VALUE time_solve_cpu;
  VALUE time_solve_gpu;

  checkCudaErrors(cusolverSpCreate(&handle));
  checkCudaErrors(cusparseCreate(&cusparseHandle));
  checkCudaErrors(cudaStreamCreate(&stream));
  checkCudaErrors(cusolverSpSetStream(handle, stream));
  checkCudaErrors(cusparseSetStream(cusparseHandle, stream));
  checkCudaErrors(cusparseCreateMatDescr(&descrA));
  checkCudaErrors(cusparseSetMatType(descrA, CUSPARSE_MATRIX_TYPE_GENERAL));

  if (baseA) {
    checkCudaErrors(cusparseSetMatIndexBase(descrA, CUSPARSE_INDEX_BASE_ONE));
  } else {
    checkCudaErrors(cusparseSetMatIndexBase(descrA, CUSPARSE_INDEX_BASE_ZERO));
  }

  checkCudaErrors(cudaMalloc((void **)&d_csrRowPtrA, sizeof(int) * (n + 1)));
  checkCudaErrors(cudaMalloc((void **)&d_csrColIndA, sizeof(int) * nnzA));
  checkCudaErrors(cudaMalloc((void **)&d_csrValA, sizeof(VALUE) * nnzA));
  checkCudaErrors(cudaMalloc((void **)&d_t_vector, sizeof(VALUE) * n));
  checkCudaErrors(cudaMalloc((void **)&d_p_vector, sizeof(VALUE) * n));

  checkCudaErrors(cudaMemcpy(d_csrRowPtrA, matrix.outerIndexPtr(),
                             sizeof(int) * (n + 1), cudaMemcpyHostToDevice));

  checkCudaErrors(cudaMemcpy(d_csrColIndA, matrix.innerIndexPtr(),
                             sizeof(int) * nnzA, cudaMemcpyHostToDevice));

  checkCudaErrors(cudaMemcpy(d_csrValA, matrix.valuePtr(), sizeof(VALUE) * nnzA,
                             cudaMemcpyHostToDevice));

  checkCudaErrors(cudaMemcpy(d_p_vector, p_vector.data(), sizeof(VALUE) * n,
                             cudaMemcpyHostToDevice));

  // solve A*x = b on GPU
  checkCudaErrors(cusolverSpScsrlsvchol(
      handle, n, nnzA, descrA, d_csrValA, d_csrRowPtrA, d_csrColIndA,
      d_p_vector, tol, reorder, d_t_vector, &singularity));
  checkCudaErrors(cudaDeviceSynchronize());

  if (singularity >= 0) {
    printf("WARNING: the matrix is singular at row %d under tol (%E)\n",
           singularity, tol);
  }

  // Copying results back to local memory
  checkCudaErrors(cudaMemcpy(t_vector.data(), d_t_vector, sizeof(VALUE) * n,
                             cudaMemcpyDeviceToHost));

  checkCudaErrors(cusolverSpDestroy(handle));
  checkCudaErrors(cusparseDestroy(cusparseHandle));
  checkCudaErrors(cudaStreamDestroy(stream));
  checkCudaErrors(cusparseDestroyMatDescr(descrA));

  checkCudaErrors(cudaFree(d_csrValA));
  checkCudaErrors(cudaFree(d_csrRowPtrA));
  checkCudaErrors(cudaFree(d_csrColIndA));
  checkCudaErrors(cudaFree(d_t_vector));

  checkCudaErrors(cudaDeviceReset());
}

void solver::gpuSolveTransient() {
  cerr << "GPU implementation doesn't support transient_ mode.";
  exit(-1);
}

#else // use CPU

#include <Eigen/PardisoSupport>
#include <unsupported/Eigen/SparseExtra>

/**
 * This solver uses Eigen library to solve the linear set of equations
 */
void solver::cpuSolveSteady(const Eigen::SparseMatrix<VALUE> &matrix,
                            const state_type &p_vector, state_type &t_vector) {
  Eigen::PardisoLLT<Eigen::SparseMatrix<VALUE>> solver;
  solver.compute(matrix);
  t_vector = solver.solve(p_vector).eval();
}

struct OdeSystem {
  OdeSystem(const state_type &p_prime_vector,
            const Eigen::SparseMatrix<VALUE> &a_matrix)
      : p_prime_vector_(p_prime_vector), a_matrix_(a_matrix) {}

  void operator()(const state_type &t_vector, state_type &dTdt, VALUE _t = 0) {
    dTdt = p_prime_vector_ - a_matrix_ * t_vector;
  }

protected:
  const state_type &p_prime_vector_;
  const Eigen::SparseMatrix<VALUE> &a_matrix_;
};

struct OdeSystemJacobi : OdeSystem {
  using OdeSystem::OdeSystem;

  void operator()(const state_type &t_vector, Eigen::SparseMatrix<VALUE> &J,
                  const VALUE &_t, state_type &dfdt) {
    J = -a_matrix_;
    dfdt.setZero(dfdt.size());
  }
};

struct OdeSystemEulerJacobi : OdeSystem {
  using OdeSystem::OdeSystem;

  void operator()(const state_type &t_vector, Eigen::SparseMatrix<VALUE> &J,
                  const VALUE &_t) {
    J = -a_matrix_;
  }
};

void solver::writeTemp(const state_type &t_vector_, const VALUE t) {
  cout << t << ":\t" << t_vector_(0) - 273.15 << " C"
       << "\n";
}

#define DT (VALUE)1e-5
#define ABS_ERROR (VALUE)1E-1
#define REL_ERROR (VALUE)1E-1

using boost::numeric::odeint::integrate_adaptive;
using boost::numeric::odeint::integrate_const;
void euler_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                  const state_type &p_prime_vector, state_type &t_vector,
                  VALUE time_start, VALUE time_end, bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit =
        integrate_adaptive(boost::numeric::odeint::euler<state_type, VALUE>(),
                           OdeSystem(p_prime_vector, a_matrix), t_vector,
                           time_start, time_end, DT);
  } else {
    steps_explicit =
        integrate_const(boost::numeric::odeint::euler<state_type, VALUE>(),
                        OdeSystem(p_prime_vector, a_matrix), t_vector,
                        time_start, time_end, DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void modified_midpoint_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                              const state_type &p_prime_vector,
                              state_type &t_vector, VALUE time_start,
                              VALUE time_end, bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        boost::numeric::odeint::modified_midpoint<state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        boost::numeric::odeint::modified_midpoint<state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void runge_kutta4_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                         const state_type &p_prime_vector, state_type &t_vector,
                         VALUE time_start, VALUE time_end,
                         bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        boost::numeric::odeint::runge_kutta4<state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        boost::numeric::odeint::runge_kutta4<state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void runge_kutta_cash_karp54_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                                    const state_type &p_prime_vector,
                                    state_type &t_vector, VALUE time_start,
                                    VALUE time_end, bool adaptive = true) {
  typedef boost::numeric::odeint::runge_kutta_cash_karp54<state_type, VALUE>
      error_stepper_type;
  size_t steps_explicit;

  if (adaptive) {
    steps_explicit = integrate_adaptive(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void runge_kutta_dopri5_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                               const state_type &p_prime_vector,
                               state_type &t_vector, VALUE time_start,
                               VALUE time_end, bool adaptive = true) {
  typedef boost::numeric::odeint::runge_kutta_dopri5<state_type, VALUE>
      error_stepper_type;
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void runge_kutta_fehlberg78_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                                   const state_type &p_prime_vector,
                                   state_type &t_vector, VALUE time_start,
                                   VALUE time_end, bool adaptive = true) {
  typedef boost::numeric::odeint::runge_kutta_fehlberg78<state_type, VALUE>
      error_stepper_type;
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        make_controlled(ABS_ERROR, REL_ERROR, error_stepper_type()),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void adams_bashforth_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                            const state_type &p_prime_vector,
                            state_type &t_vector, VALUE time_start,
                            VALUE time_end, bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        boost::numeric::odeint::adams_bashforth<4, state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        boost::numeric::odeint::adams_bashforth<4, state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void adams_bashforth_moulton_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                                    const state_type &p_prime_vector,
                                    state_type &t_vector, VALUE time_start,
                                    VALUE time_end, bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = integrate_adaptive(
        boost::numeric::odeint::adams_bashforth_moulton<4, state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  } else {
    steps_explicit = integrate_const(
        boost::numeric::odeint::adams_bashforth_moulton<4, state_type, VALUE>(),
        OdeSystem(p_prime_vector, a_matrix), t_vector, time_start, time_end,
        DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void bulirsch_stoer_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                           const state_type &p_prime_vector,
                           state_type &t_vector, VALUE time_start,
                           VALUE time_end, bool adaptive = true) {
  boost::numeric::odeint::bulirsch_stoer_dense_out<state_type, VALUE> stepper(
      5E-1, 1E-1);
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit =
        integrate_adaptive(stepper, OdeSystem(p_prime_vector, a_matrix),
                           t_vector, time_start, time_end, DT);
  } else {
    steps_explicit =
        integrate_const(stepper, OdeSystem(p_prime_vector, a_matrix), t_vector,
                        time_start, time_end, DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void implicit_euler_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                           const state_type &p_prime_vector,
                           state_type &t_vector, VALUE time_start,
                           VALUE time_end, bool adaptive = true) {
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = boost::numeric::odeint::integrate_adaptive(
        boost::numeric::odeint::implicit_euler<VALUE>(ABS_ERROR),
        std::make_pair(OdeSystem(p_prime_vector, a_matrix),
                       OdeSystemEulerJacobi(p_prime_vector, a_matrix)),
        t_vector, time_start, time_end, DT);
  } else {
    steps_explicit = boost::numeric::odeint::integrate_const(
        boost::numeric::odeint::implicit_euler<VALUE>(ABS_ERROR),
        std::make_pair(OdeSystem(p_prime_vector, a_matrix),
                       OdeSystemEulerJacobi(p_prime_vector, a_matrix)),
        t_vector, time_start, time_end, DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void rosenbrock_method(const Eigen::SparseMatrix<VALUE> &a_matrix,
                       const state_type &p_prime_vector, state_type &t_vector,
                       VALUE time_start, VALUE time_end, bool adaptive = true) {
  typedef boost::numeric::odeint::rosenbrock4_controller<
      boost::numeric::odeint::rosenbrock4<VALUE>>
      controlled_stepper_type;
  size_t steps_explicit;
  if (adaptive) {
    steps_explicit = boost::numeric::odeint::integrate_adaptive(
        controlled_stepper_type(ABS_ERROR, REL_ERROR),
        std::make_pair(OdeSystem(p_prime_vector, a_matrix),
                       OdeSystemJacobi(p_prime_vector, a_matrix)),
        t_vector, time_start, time_end, DT);
  } else {
    steps_explicit = boost::numeric::odeint::integrate_const(
        controlled_stepper_type(ABS_ERROR, REL_ERROR),
        std::make_pair(OdeSystem(p_prime_vector, a_matrix),
                       OdeSystemJacobi(p_prime_vector, a_matrix)),
        t_vector, time_start, time_end, DT);
  }
  cout << "Number of steps: " << steps_explicit << "\n";
}

void solver::cpuSolveTransient(
    const Eigen::SparseMatrix<VALUE> &a_matrix,
    const Eigen::DiagonalMatrix<VALUE, Eigen::Dynamic, Eigen::Dynamic> &invC,
    const state_type &p_vector, state_type &t_vector, VALUE time_start,
    VALUE time_end) {

  // utils::dumpVector(invC.diagonal(), "inv_c.mtx");
  /* Calculating P' = C^-1 * P  */
  state_type p_prime_vector = invC * p_vector;

  const int k = 21;

  switch (k) {
  case 0:
    euler_method(a_matrix, p_prime_vector, t_vector, time_start, time_end,
                 false);
    break;
  case 1:
    euler_method(a_matrix, p_prime_vector, t_vector, time_start, time_end,
                 true);
    break;
  case 2:
    modified_midpoint_method(a_matrix, p_prime_vector, t_vector, time_start,
                             time_end, false);
    break;
  case 3:
    modified_midpoint_method(a_matrix, p_prime_vector, t_vector, time_start,
                             time_end, true);
    break;
  case 4:
    runge_kutta4_method(a_matrix, p_prime_vector, t_vector, time_start,
                        time_end, false);
    break;
  case 5:
    runge_kutta4_method(a_matrix, p_prime_vector, t_vector, time_start,
                        time_end, true);
    break;
  case 6:
    runge_kutta_cash_karp54_method(a_matrix, p_prime_vector, t_vector,
                                   time_start, time_end, false);
    break;
  case 7:
    runge_kutta_cash_karp54_method(a_matrix, p_prime_vector, t_vector,
                                   time_start, time_end, true);
    break;
  case 8:
    runge_kutta_dopri5_method(a_matrix, p_prime_vector, t_vector, time_start,
                              time_end, false);
    break;
  case 9:
    runge_kutta_dopri5_method(a_matrix, p_prime_vector, t_vector, time_start,
                              time_end, true);
    break;
  case 10:
    runge_kutta_fehlberg78_method(a_matrix, p_prime_vector, t_vector,
                                  time_start, time_end, false);
    break;
  case 11:
    runge_kutta_fehlberg78_method(a_matrix, p_prime_vector, t_vector,
                                  time_start, time_end, true);
    break;
  case 12:
    adams_bashforth_method(a_matrix, p_prime_vector, t_vector, time_start,
                           time_end, false);
    break;
  case 13:
    adams_bashforth_method(a_matrix, p_prime_vector, t_vector, time_start,
                           time_end, true);
    break;
  case 14:
    adams_bashforth_moulton_method(a_matrix, p_prime_vector, t_vector,
                                   time_start, time_end, false);
    break;
  case 15:
    adams_bashforth_moulton_method(a_matrix, p_prime_vector, t_vector,
                                   time_start, time_end, true);
    break;
  case 16:
    bulirsch_stoer_method(a_matrix, p_prime_vector, t_vector, time_start,
                          time_end, false);
    break;
  case 17:
    bulirsch_stoer_method(a_matrix, p_prime_vector, t_vector, time_start,
                          time_end, true);
    break;
  case 18:
    implicit_euler_method(a_matrix, p_prime_vector, t_vector, time_start,
                          time_end, false);
    break;
  case 19:
    implicit_euler_method(a_matrix, p_prime_vector, t_vector, time_start,
                          time_end, true);
    break;
  case 20:
    rosenbrock_method(a_matrix, p_prime_vector, t_vector, time_start, time_end,
                      false);
    break;
  case 21:
    rosenbrock_method(a_matrix, p_prime_vector, t_vector, time_start, time_end,
                      true);
    break;
  default:
    exit(-1);
  }
}

#endif
