#pragma once
#ifndef VOLREND_COMMON_93D99C8D_E8CA_4FFE_9716_D8237925F910
#define VOLREND_COMMON_93D99C8D_E8CA_4FFE_9716_D8237925F910

#define VOLREND_VERSION_MAJOR 0
#define VOLREND_VERSION_MINOR 0
#define VOLREND_VERSION_PATCH 1

// #define VOLREND_CUDA
// #define VOLREND_PNG

#ifdef __CUDACC__
#define VOLREND_COMMON_FUNCTION __host__ __device__ __inline__
#define VOLREND_RESTRICT __restrict__
#define VOLREND_MIN(a, b) min(a, b)
#define VOLREND_MAX(a, b) max(a, b)
#else
#define VOLREND_COMMON_FUNCTION
#define VOLREND_RESTRICT
#define VOLREND_MIN(a, b) ((a < b) ? a : b)
#define VOLREND_MAX(a, b) ((a > b) ? a : b)
#endif

#endif  // ifndef VOLREND_COMMON_93D99C8D_E8CA_4FFE_9716_D8237925F910
