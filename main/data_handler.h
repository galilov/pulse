//
// Created by Alexander Galilov <alexander.galilov@gmail.com>
// on 10/19/19.
//

#ifndef I2S_ADC_DAC_DATA_HANDLER_H
#define I2S_ADC_DAC_DATA_HANDLER_H

#include <stddef.h>
//enable display buffer for debug
#define BUF_DEBUG     (0)
// Sample rate
#define SAMPLE_RATE   (6000)
// Low-pass filter, Hz (-3db cut-off)
#define LOW_PASS_FILTER_FREQ (300)

typedef struct {
    float coeffPulsation;
    int min, max;
    bool isError;
#if BUF_DEBUG
    int* data;
    size_t dataSize;
#endif
} ProcessResult;

#ifdef __cplusplus
extern "C" {
#endif
extern ProcessResult process_data(const void *pData, size_t size, bool use_filter);
#ifdef __cplusplus
}
#endif

#endif //I2S_ADC_DAC_DATA_HANDLER_H
