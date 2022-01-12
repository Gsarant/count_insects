
#ifndef TF_MICRO_H
#define TF_MICRO_H


#include "sdkconfig.h"
#include "esp_attr.h"

#include <esp_log.h>

//#include "tensorflow/lite/version.h"
//#include "tensorflow/lite/c/common.h"
//#include "normalisation_lookup.h"
//#include "image_util.h"

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif
//bool is_init_tf;
//bool  init_tf(void);

int getPredict(const uint8_t *image,const size_t width,const size_t height,const size_t channel);
#ifdef __cplusplus
}
#endif


#endif //TF_MICRO_H

//#include "tfmicro.cc"
