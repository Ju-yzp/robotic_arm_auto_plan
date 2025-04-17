#include <algorithm>
#include <cstdint>
#include <cstring>

// #include <memory>
#include <emmintrin.h>

#include <opencv2/core/cvdef.h>

#include <semi_global_matching/sgm_util.h>

namespace sgm_util {
void Util::computeAggregationHorizontal(semi_global_matching::SemiGlobalMatching *sgm,uint8_t *cost_aggr,bool forward)
{
    // 获得图像参数以及初始代价参数
    const int32_t width = sgm->width_;
    const int32_t height = sgm->height_;
    const int32_t min_disparity = sgm->option_.get_min_disparity();
    const int32_t disparity_range = sgm->option_.get_disparity_range();
    const int32_t p1 = sgm->option_.get_factor();
    const int32_t p2_init = sgm->option_.get_penalty_factor();
    const int32_t direction = forward ? 1 : -1;

    // 转换一下图像资源指针
    uchar *ptr = const_cast<uchar *>(sgm->left_img_->data);
    uint8_t *img = static_cast<uint8_t *>(ptr);

    // 用于保存上一次的路径代价
    std::vector<uint8_t> last_cost_path(disparity_range+2,UINT8_MAX);

    // 上一次的最小代价
    uint8_t min_cost_last_path;

    // TODO:嵌套循环内尽量少使用变量,尽可能的放在循环外面,避免频繁的压栈和入栈
    int32_t offest = direction * disparity_range;
    uint8_t gray,last_gray,min_cost ;

    for(int32_t i = 0u; i < height;i++)
    {
        auto cost_aggr_row = direction ? 
                        (cost_aggr + i * width * disparity_range) : 
                        (cost_aggr + i  * width * disparity_range + (width - 1) *disparity_range);
        auto cost_init_row = forward ? 
                        (sgm->cost_init_ + i * width * disparity_range) : 
                        (sgm->cost_init_ + i * width * disparity_range + ((width - 1) *disparity_range));
        auto img_row = forward ? 
                                  (img + i * width):
                                  (img + i * width + width - 1);

        // 起始图像灰度值
        gray = *img_row;
        last_gray = *img_row;

        // 初始化路径的起始代价
        std::memcpy(cost_aggr_row,cost_init_row,disparity_range * sizeof(uint8_t));
        std::memcpy(&last_cost_path[1],cost_init_row,disparity_range * sizeof(uint8_t));

        cost_init_row += offest;
        cost_aggr_row += offest;
        img_row       += direction;

        // 上一个像素的最小代价值
        uint8_t last_path_min_cost = UINT8_MAX;
        for(auto &cost:last_cost_path)
        {
            last_path_min_cost = std::min(cost,last_path_min_cost);
        }

        for(int32_t j = 0; j < width - 1;j++)
        {
            gray = *img_row;
            min_cost = UINT8_MAX;

            for(int32_t d = 0; d < disparity_range ;d++)
            {
                const uint8_t  cost = cost_init_row[d];
                const uint16_t   l1 = last_cost_path[d+1];
                const uint16_t   l2 = last_cost_path[d]+p1;
                const uint16_t   l3 = last_cost_path[d+2] + p1;

                // 这里+1是为了避免除数为0引发抛出异常
                const uint16_t   l4 = last_path_min_cost + std::max(p1,p2_init/(std::abs(gray - last_gray )+ 1));

                const uint8_t cost_s = cost + static_cast<uint8_t>(std::min(std::min(l1,l2),std::min(l3,l4)))-last_path_min_cost;

                cost_aggr_row[d] = cost_s;
                min_cost = std::min(min_cost,cost_s);
            }

            // 拷贝当前的路径代价作为下一次的
            last_path_min_cost = min_cost;
            std::memcpy(&last_cost_path[1],cost_aggr_row,disparity_range * sizeof(uint8_t));

            //
            cost_init_row += offest;
            cost_aggr_row += offest;
            img_row       += direction;

            last_gray = gray;
        }
    }
}

void Util::computeAggregationVertical(semi_global_matching::SemiGlobalMatching *sgm,uint8_t *cost_aggr,bool forward)
{
    // 获得图像参数以及初始代价参数
    const int32_t width = sgm->width_;
    const int32_t height = sgm->height_;
    const int32_t min_disparity = sgm->option_.get_min_disparity();
    const int32_t disparity_range = sgm->option_.get_disparity_range();
    const int32_t p1 = sgm->option_.get_factor();
    const int32_t p2_init = sgm->option_.get_penalty_factor();
    const int32_t direction = forward ? 1 : -1;

    // 转换一下图像资源指针
    uchar *ptr = const_cast<uchar *>(sgm->left_img_->data);
    uint8_t *img = static_cast<uint8_t *>(ptr);

    // 用于保存上一次的路径代价
    std::vector<uint8_t> last_cost_path(disparity_range+2,UINT8_MAX);

    // 上一次的最小代价
    uint8_t min_cost_last_path;

    // TODO:嵌套循环内尽量少使用变量,尽可能的放在循环外面,避免频繁的压栈和入栈
    int32_t offest = direction * width * disparity_range;
    uint8_t gray,last_gray;    

    for(int32_t i = 0u; i < width ;i ++)
    {
        auto cost_aggr_col = forward ?
                             (cost_aggr + i * disparity_range):
                             (cost_aggr + (height - 1)* width *disparity_range + i * disparity_range);

        auto cost_init_col = forward ?
                             (sgm->cost_init_ + i * disparity_range):
                             (sgm->cost_init_ + (height - 1)* width *disparity_range + i * disparity_range);

        auto img_col       = forward ?
                                       (img +  i):
                                       (img + (height - 1) * width + i);


        
        gray = *img_col;
        last_gray = *img_col;

        std::memcpy(&last_cost_path[0],cost_init_col,disparity_range * sizeof(uint8_t));
        std::memcpy(cost_aggr_col, cost_init_col,disparity_range * sizeof(uint8_t));

        cost_init_col += offest;
        cost_aggr_col += offest;
        img_col       += direction * width;

        // 上一个像素的最小代价值
        uint8_t last_path_min_cost = UINT8_MAX;
        for(auto &cost:last_cost_path)
        {
            last_path_min_cost = std::min(cost,last_path_min_cost);
        }

        for(int32_t j = 0u; j < height - 1; j++)
        {
            gray = *img_col;
            uint8_t min_cost = UINT8_MAX;

            for(int32_t d = 0; d < disparity_range ;d++)
            {
                const uint8_t  cost = cost_init_col[d];
                const uint16_t   l1 = last_cost_path[d+1];
                const uint16_t   l2 = last_cost_path[d]+p1;
                const uint16_t   l3 = last_cost_path[d+2] + p1;

                // 这里+1是为了避免除数为0引发抛出异常
                const uint16_t   l4 = last_path_min_cost + std::max(p1,p2_init/(std::abs(gray - last_gray )+ 1));

                const uint8_t cost_s = cost + static_cast<uint8_t>(std::min(std::min(l1,l2),std::min(l3,l4)))-last_path_min_cost;

                cost_aggr_col[d] = cost_s;
                min_cost = std::min(min_cost,cost_s);
            }

            // 拷贝当前的路径代价作为下一次的
            min_cost_last_path = min_cost;
            std::memcpy(&last_cost_path[0],cost_aggr_col,disparity_range * sizeof(uint8_t));

            //
            cost_init_col += offest;
            cost_aggr_col += offest;
            img_col       += direction * width;

            last_gray = gray;
        }
    }
}

// void Util::computeAggregationVertical(semi_global_matching::SemiGlobalMatching *sgm)
// {
    
// }

uint8_t Util::Hamming(uint32_t x, uint32_t y)
{
    uint32_t dist,value = x ^ y;
    while (value) {
           dist++;
           value &= value - 1;
    }
    return static_cast<uint8_t>(dist);
}
} 