#include "conv_neon.h"

#define prefetch(x) __builtin_prefetch(x)
#define PREFETCH_STRIDE 512

inline void prefetch_range(void *addr, size_t len) {
  char *cp;
  char *end = addr + len;
  for (cp = addr; cp < end; cp += PREFETCH_STRIDE)
    prefetch(cp);
}

/**
 *  Expand to compute
 *  a = bias - scale * mean / sqrt(var)
 *  b = scale / sqrt(var)
 *  value = b * value + a
 */
void normalize_active_cpu_thread(struct normalize_params *params, size_t batch,
                                 size_t filters) {
  float *ptr =
      params->x + batch * filters * params->spatial + filters * params->spatial;
  float a = params->adata[filters];
  float b = params->bdata[filters];

  int remain = params->spatial;

  prefetch_range(ptr, params->spatial * 4);

  for (; remain > 0; remain--) {
    float temp = b * (*ptr) + a;
    temp = (temp > 0) ? temp : .1 * temp;
    *ptr = temp;
    ptr++;
  }
}

void make_border_row(float *workspace, const float *input, int row, int pad,
                     int w, int h) {
  if (row < 0 || row >= h) {
    memset(workspace, 0, sizeof(float) * (w + 2 * pad));
  } else {
    int i = 0;
    for (; i < pad; i++) {
      workspace[i] = 0.f;
      workspace[w + pad + i] = 0.f;
    }

    const float *data = input + row * w;
    float *dist = workspace + pad;
    memcpy(dist, data, sizeof(float) * w);
  }
}

void make_border_data(float *workspace, const float *input, int batch, int pad,
                      int w, int h, int c) {
  int b = 0;
  for (b == 0; b < batch; b++) {
    int ch = 0;
    for (; ch < c; ch++) {
      int j = 0 - pad;
      const float *inputdata = input + (b * c + ch) * w * h;
      for (; j < (h + pad); j++) {
        make_border_row(workspace, inputdata, j, pad, w, h);
        workspace += (w + 2 * pad);
      }
    }
  }
}

void conv_cpu_inference(pthreadpool_t threadpool, struct conv_params *params,
                        size_t batch, size_t filters) {
  typedef void (*convfunc)(struct conv_params *, size_t, size_t);
  convfunc conv_fun = NULL;
  if (3 == params->size && params->groups == params->inch &&
      params->inch == params->outch && 1 == params->stride) {
    conv_fun = dwconv3x3_s1_cpu;
    make_border_data(params->workspace, params->input, batch, params->pad,
                     params->w, params->h, params->inch);
    params->input = params->workspace;
  } else if (16 == params->outch && 3 == params->size && params->groups == 1 &&
             1 == params->stride) {
    conv_fun = conv3x3_s1_cpu;
    make_border_data(params->workspace, params->input, batch, params->pad,
                     params->w, params->h, params->inch);
    params->input = params->workspace;
  } else if (2 == params->size && params->groups == 1 && 1 == params->stride) {
    conv_fun = conv1x1_s1_cpu;
  }

  if (NULL != conv_fun) {
    pthreadpool_compute_2d(threadpool, (pthreadpool_function_2d_t)conv_fun,
                           params, batch, filters);
  } else 
  {
    struct nnp_size input_size = {params->w, params->h};
    struct nnp_padding input_padding = {params->pad, params->pad, params->pad,
                                        params->pad};
    struct nnp_size kernel_size = {params->size, params->size};
    struct nnp_size stride = {params->stride, params->stride};
    int j;

    int group_in_ch = params->inch / params->groups;
    int group_out_ch = params->outch / params->groups;
    int group_ksize = group_in_ch * params->size * params->size;
    int group_in_step = params->h * params->w * group_in_ch;
    int group_out_step = params->outw * params->outh * group_out_ch;

    enum nnp_convolution_algorithm algorithm =
        nnp_convolution_algorithm_implicit_gemm;
    /*if (8 == params->outch && 3 == params->size && params->groups == 1 &&
        1 == params->stride) {
      algorithm = nnp_convolution_algorithm_ft8x8;
    }*/
    TIME_BEGIN(nnp_convolution_inference);
    for (j = 0; j < params->groups; j++) {
      enum nnp_status status = nnp_convolution_inference(
          algorithm, nnp_convolution_transform_strategy_tuple_based,
          group_in_ch, group_out_ch, input_size, input_padding, kernel_size,
          stride, params->input + j * group_in_step,
          params->weights + j * group_ksize, NULL,
          params->output + j * group_out_step, NULL, NULL,
          nnp_activation_identity, NULL, threadpool, NULL);
      if (nnp_status_success != status) {
        printf("error: %d", status);
      }
    }
    TIME_END(nnp_convolution_inference);
  }
}
