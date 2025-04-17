#ifndef SEMI_GLOBAL_MATCHING_SEMI_GLOBAL_MATCHING_H_
#define SEMI_GLOBAL_MATCHING_SEMI_GLOBAL_MATCHING_H_

#include <cstdint>

#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <semi_global_matching/option.h>

#include <memory>

// 
namespace sgm_util {
    class Util;
}

namespace semi_global_matching {

class SemiGlobalMatching
{

public:

friend sgm_util::Util;

explicit SemiGlobalMatching(MatchOption &option);

~SemiGlobalMatching();

void match(std::shared_ptr<cv::Mat> left_img, std::shared_ptr<cv::Mat> right_img);

void display();

private:

void initialize(const int width,const int height);

void releaseResource();

void computeCensus(const int flag);

void computeCost();

void costAggregation();

uint32_t *left_census_;
uint32_t *right_census_;

uint8_t *cost_init_;

uint16_t *cost_aggr_;


std::vector<uint8_t *> cost_aggrs_;

std::shared_ptr<cv::Mat> left_img_;
std::shared_ptr<cv::Mat> right_img_;

// std::vector<
// 
MatchOption option_;

// 图像信息
int width_;
int height_;
bool initialized_;
};

}
#endif