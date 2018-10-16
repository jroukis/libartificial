#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS // to disable deprecation warnings
#include "../../clblast/include/clblast_c.h"

#include "../headers/gpu.h"

#define KRED  "\x1B[31m"
#define RESET "\033[0m"

double ***gpu_feedforward_cache(const int *rows, const int *columns_Y, const int *columns_X, const int *layers,
                                double *X, double **w,
                                const int *nodes, const int *funcs,
                                cl_context *context, cl_device_id *device)
{
  // l is for layers
  // i is for each row * column of X, Y
  int l = (*layers);
  int i = (*rows) * (*columns_Y);
  
  // feeds at every layer
  double ***Z = malloc(2 * sizeof(double **));
  if (Z) {
    Z[0] = malloc(((*layers) + 1) * sizeof(double *));
    Z[1] = malloc(((*layers) + 1) * sizeof(double *));
    if (Z[0] && Z[1]) {
      Z[0][l] = malloc(i * sizeof(double));
      Z[1][l] = malloc(i * sizeof(double));
      if (Z[0][l] && Z[1][l]) {
        for (l = 0; l < (*layers); l++) {
          i = (*rows) * nodes[l];
          Z[0][l] = malloc(i * sizeof(double));
          Z[1][l] = malloc(i * sizeof(double));
          if (Z[0][l] && Z[1][l]) {
            memset(Z[0][l], 0.0, i * sizeof(double));
            memset(Z[1][l], 0.0, i * sizeof(double));
          } else {
            printf(KRED "\nCould not allocate Zs. Aborting...\n" RESET);
            free(Z[0]);
            free(Z[1]);
            free(Z);
            abort();
          }
        }
      } else {
        printf(KRED "\nCould not allocate Zs. Aborting...\n" RESET);
        free(Z[0]);
        free(Z[1]);
        free(Z);
        abort();
      }
    } else {
      printf(KRED "\nCould not allocate Zs. Aborting...\n" RESET);
      free(Z);
      abort();
    }
  } else {
    printf(KRED "\nCould not allocate Zs. Aborting...\n" RESET);
    abort();
  }
  
  // Directly manipulates Z
  gpu_feedforward_update(rows, columns_Y, columns_X, layers, Z, X, w, nodes, funcs, context, device);
  return Z;
}

