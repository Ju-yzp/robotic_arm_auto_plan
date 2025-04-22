#ifndef SEMI_GLOBAL_MATCHING_SGM_UTIL_H_
#define SEMI_GLOBAL_MATCHING_SGM_UTIL_H_

#include <cstdint>
#include <semi_global_matching/SemiGlobalMatching.h>

namespace sgm_util {
class Util
{
public:
// 在水平方向的像素进行代价聚合
static void computeAggregationHorizontal(semi_global_matching::SemiGlobalMatching *sgm ,uint8_t *cost,bool forward);

// 在竖直方向的像素进行代价聚合
static void computeAggregationVertical(semi_global_matching::SemiGlobalMatching *sgm,uint8_t *cost,bool forward);

static uint8_t Hamming(uint32_t x, uint32_t y);
};
}
#endif 