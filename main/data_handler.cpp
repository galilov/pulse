//
// Created by Alexander Galilov <alexander.galilov@gmail.com>
// on 10/19/19.
//
#include<bits/stdc++.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "data_handler.h"

inline int toNormalValue(const unsigned short &src) {
    return (int) ((~src) & 0x0fff);
}

typedef std::vector<int> TDataVector;

static double out = 0;
static double a;

void low_pass_filter_init(unsigned int avg) {
    out = avg;
    auto rc = 0.5 / LOW_PASS_FILTER_FREQ / M_PI;
    a = (1.0 / SAMPLE_RATE) / (rc + 1.0 / SAMPLE_RATE);
}

int low_pass_filter(int data) {
    // https://en.wikipedia.org/wiki/Low-pass_filter
    out = out + a * (data - out);
    return (int) round(out);
}

void toFilteredDataVector(const unsigned short *data, size_t size, TDataVector &target, bool use_filter) {
    int avg = 0;
    for (size_t index = 0; index < size; index++) {
        avg += toNormalValue(data[index]);
    }
    avg /= size;
    low_pass_filter_init(avg);
    target.clear();
    target.reserve(size);
    for (size_t index = 0; index < size; index++) {
        int sample = toNormalValue(data[index]);
        target.emplace_back(use_filter ? low_pass_filter(sample) : sample);
    }
}

TDataVector data;

ProcessResult process_data(const void *pData, size_t size, bool use_filter) {
    ProcessResult result;
    result.coeffPulsation = 0;
    auto pTypedData = reinterpret_cast<const unsigned short *>(pData);
    toFilteredDataVector(pTypedData, size / sizeof(*pTypedData), data, use_filter);
    auto max = *std::max_element(data.cbegin(), data.cend());
    auto min = *std::min_element(data.cbegin(), data.cend());
    result.max = max;
    result.min = min;
    auto sum = std::accumulate(data.cbegin(), data.cend(), 0);
    auto t = data.size();
#if BUF_DEBUG
    result.data = &data[0];
    result.dataSize = data.size();
#endif
    if (sum == 0 || t == 0 || result.min > 3000 || result.max < 256 || result.min > 3000 || result.min < 256) {
        result.coeffPulsation = 0;
        result.isError = true;
        return result;
    }
    // http://ekosf.ru/stati/525-pulsacii
    result.coeffPulsation = (max - min) * 100 / (2.0 * sum / t);
    result.isError = false;
    return result;
}
