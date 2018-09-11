#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS // to disable deprecation warnings
#include "../../clblast/include/clblast_c.h"

#include "../headers/utils.h"

// DELTAS FOR EVERY TYPE OF GRADIENT DESCENT
// (batch => rows = all rows)
// (stochastic => rows = 1)
// (mini-batch => rows = rows of mini set)

void delta_gd_gpu(double **deltas, const size_t rows, const size_t columns_Y, const int layers,
                  const double *Y, double ***Z, double ***wb,
                  const int nodes[layers], char funcs[layers+1][30],
                  const cl_device_id device
                 )
{
  int l, i;
  int for_helper = rows * columns_Y;
  // Deltas filling
  //////////////////////////////////////////////////////////////////
  // Gradient of layer's unactivated output
  double **help_1 = malloc(layers * sizeof(double *));
  // Product of next layer's transposed weights and deltas
  double **help_2 = malloc(layers * sizeof(double *));
  // We do not need them at the output layer
  //////////////////////////////////////////////////////////////////
  
  // Last layer
  switch (strcmp(funcs[layers], "linear")) {
    case 0:
      i = for_helper - 1;
      do {
        deltas[layers][i] = Z[1][layers][i] - Y[i];
        i--;
      } while (i >= 0);
      break;
    default:
      gradient(deltas[layers], Z[0][layers], for_helper, funcs[layers]);
      i = for_helper - 1;
      do {
        deltas[layers][i] = (Z[1][layers][i] - Y[i]) * deltas[layers][i];
        i--;
      } while (i >= 0);
      break;
  }
  
  // Creates the OpenCL context, queue, and an event
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);
  cl_event event = NULL;
  
  cl_mem device_a, device_b, device_c;
  
  l = layers - 1;
  do {
    for_helper = rows * nodes[l];
    
    help_1[l] = malloc(for_helper * sizeof(double));
    help_2[l] = malloc(for_helper * sizeof(double));
    
    gradient(help_1[l], Z[0][l], for_helper, funcs[l]);
    i = for_helper;
    // Layers backwards
    do {
      help_2[l][i--] = 0.0;
    } while (i >= 0);
    
    switch (l == layers - 1) {
      // True
      case 1:
        // Copy the matrices to the device
        device_a = clCreateBuffer(context, CL_MEM_READ_WRITE, rows * columns_Y * sizeof(double), NULL, NULL);
        device_b = clCreateBuffer(context, CL_MEM_READ_WRITE, nodes[l] * columns_Y * sizeof(double), NULL, NULL);
        device_c = clCreateBuffer(context, CL_MEM_READ_WRITE, rows * nodes[l] * sizeof(double), NULL, NULL);
        clEnqueueWriteBuffer(queue, device_a, CL_TRUE, 0, rows * columns_Y * sizeof(double), deltas[l+1], 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, device_b, CL_TRUE, 0, nodes[l] * columns_Y * sizeof(double), wb[0][l+1], 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, device_c, CL_TRUE, 0, rows * nodes[l] * sizeof(double), help_2[l], 0, NULL, NULL);
        
        CLBlastStatusCode status = CLBlastDgemm(CLBlastLayoutRowMajor, CLBlastTransposeNo, CLBlastTransposeYes,
                                                rows, nodes[l], columns_Y,
                                                1.0,
                                                device_a, 0, columns_Y,
                                                device_b, 0, columns_Y,
                                                0.0,
                                                device_c, 0, nodes[l],
                                                &queue, &event);
        
        // Wait for completion
        if (status == CLBlastSuccess) {
          clWaitForEvents(1, &event);
          clEnqueueReadBuffer(queue, device_c, CL_TRUE, 0, rows * nodes[l] * sizeof(double),help_2[l],0,NULL,NULL);
          clReleaseEvent(event);
        }
        
        // Hadamard product
        i = for_helper;
        do {
          deltas[l][i] = help_1[l][i] * help_2[l][i];
          i--;
        } while (i >= 0);
        
        free(help_2[l]);
        free(help_1[l]);
        l--;
        continue;
      // False
      default:
        // find help_2
        // Copy the matrices to the device
        device_a = clCreateBuffer(context, CL_MEM_READ_WRITE, rows * nodes[l+1] * sizeof(double), NULL, NULL);
        device_b = clCreateBuffer(context, CL_MEM_READ_WRITE, nodes[l] * nodes[l+1] * sizeof(double), NULL, NULL);
        device_c = clCreateBuffer(context, CL_MEM_READ_WRITE, rows * nodes[l] * sizeof(double), NULL, NULL);
        clEnqueueWriteBuffer(queue, device_a, CL_TRUE, 0, rows * nodes[l+1] * sizeof(double), deltas[l+1], 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, device_b, CL_TRUE, 0, nodes[l] * nodes[l+1] * sizeof(double), wb[0][l+1], 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, device_c, CL_TRUE, 0, rows * nodes[l] * sizeof(double), help_2[l], 0, NULL, NULL);
        
        status = CLBlastDgemm(CLBlastLayoutRowMajor, CLBlastTransposeNo, CLBlastTransposeYes,
                              rows, nodes[l], nodes[l+1],
                              1.0,
                              device_a, 0, nodes[l+1],
                              device_b, 0, nodes[l+1],
                              0.0,
                              device_c, 0, nodes[l],
                              &queue, &event);
        
        // Wait for completion
        if (status == CLBlastSuccess) {
          clWaitForEvents(1, &event);
          clEnqueueReadBuffer(queue, device_c, CL_TRUE, 0, rows * nodes[l] * sizeof(double),help_2[l],0,NULL,NULL);
          clReleaseEvent(event);
        }
        
        // Hadamard product
        i = for_helper;
        do {
          deltas[l][i] = help_1[l][i] * help_2[l][i];
          i--;
        } while (i >= 0);
        
        free(help_2[l]);
        free(help_1[l]);
        l--;
        continue;
    }
  } while (l >= 0);
  // Freedom final
  clReleaseMemObject(device_a);
  clReleaseMemObject(device_b);
  clReleaseMemObject(device_c);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  free(help_2);
  free(help_1);
}
