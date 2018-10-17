#ifndef utils_h__
#define utils_h__

extern int *name2int(const int layers, char funcs[layers+1][30]);
extern void activate(double *restrict X_active, const double *restrict X,
                     const int *restrict rows, const int *restrict cols,
                     const int *restrict function);
extern void gradient(double *restrict X_graded, const double *restrict X,
                     const int *restrict rows, const int *restrict cols,
                     const int *restrict function);

extern double rand_normal(const double mu, const double sigma);

extern double rmse(const int *restrict rows, const int *restrict columns_Y,
                   const double *restrict Y, const double *restrict Z_active);
extern double xentropy(const int *restrict rows, const int *restrict columns_Y,
                       const double *restrict Y, const double *restrict Z_active);

// Convolution utility
extern int **im2col(int ***images,const int no_of_images,
                    const size_t img_width, const size_t img_height, const size_t img_channels,
                    const size_t spatial, // width and height of weights
                    const size_t stride, // (img_width - spatial + 2 * padding)/stride should be int
                    const size_t padding, // Zeros around
                    const size_t delete_originals // 0 if no, 1 if yes (keep only vectorized in memory)
);

// Save trained wb
extern void save_w(double **weights, const int layers, const int nodes[layers],
                   const int columns_Y, const int columns_X);
// Freedom
extern void delete_Z(const int layers, double ***Z);
extern void delete_img_vector(int **images, const size_t no_of_images);

#endif  // utils_h__
