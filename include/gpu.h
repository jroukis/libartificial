#ifndef gpu_h__
#define gpu_h__

#ifdef WIN32
  #if defined(COMPILING_DLL)
    #define PUBLIC_API __declspec(dllexport)
  #else
    #define PUBLIC_API __declspec(dllimport)
  #endif
#else
  #define PUBLIC_API
#endif
/*
#ifdef __APPLE__
  #include "OpenCL/opencl.h"
#else
  #include "CL/cl.h"
#endif*/

extern void PUBLIC_API randomize(double *X, const int rows, const int columns_X);
extern void PUBLIC_API normalize(double *X, const int rows, const int columns_X);

extern double PUBLIC_API **init_w(const double variance, const int layers,
                                  const int nodes[layers], char funcs[layers+1][30],
                                  const int columns_Y, const int columns_X);

// Load pretrained wb files
extern double PUBLIC_API **load_w(const int layers, const int nodes[layers], const int columns_Y, const int columns_X);

// Free wb
extern void PUBLIC_API delete_w(const int layers, double **weights);

// Actual perceptron
extern double PUBLIC_API *gpu_feedforward_predict(const int rows, const int columns_Y,
                                                  const int columns_X, const int layers,
                                                  double *X, double **weights,
                                                  const int nodes[layers], char funcs[layers + 1][30]);

extern void PUBLIC_API gpu_gd_train(const int rows, const int columns_Y, const int columns_X,
                                    const int batch, const int layers,
                                    const int nodes[layers],
                                    double *Y, double *X, double **weights,
                                    char funcs[layers + 1][30],
                                    const double learning_rate, const int epochs);
#endif  // gpu_h__
