#include <cassert>
#include <cstdint>

#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui.hpp>

#include <immintrin.h>

#include <semi_global_matching/SemiGlobalMatching.h>
#include <semi_global_matching/sgm_util.h>

#define RELEASE_RESOURCE(ptr) if(ptr != nullptr)\
                                 delete[] ptr;

namespace semi_global_matching {

SemiGlobalMatching::SemiGlobalMatching(MatchOption &option)
:option_(option),
left_census_(nullptr),
right_census_(nullptr),
initialized_(false)
{}

SemiGlobalMatching::~SemiGlobalMatching()
{
    releaseResource();
}

void SemiGlobalMatching::match(std::shared_ptr<cv::Mat> left_img, std::shared_ptr<cv::Mat> right_img)
{
    assert((left_img->cols == right_img->cols) && (left_img->rows == right_img->rows));
    assert(left_img->type() == CV_8UC1 && right_img->type() == CV_8UC1);

    // 如果进行初始化操作，就检查图像大小是否发生变化，
    // 有就释放之前分配的内存，并重新分配内存
    const int width = left_img->cols;
    const int height = left_img->rows;
    if(initialized_)
    {
        if(width != width_ || height != height_)
        {
            initialized_ = false;
            releaseResource();
            initialize(width,height);
        }       
    } else {
        initialize(width,height);
    }

    left_img_  = left_img;
    right_img_ = right_img;
    // 计算
    auto start = std::chrono::system_clock::now();
    computeCensus(0);
    computeCensus(1);
    // std::thread p1(&SemiGlobalMatching::computeCensus,this,0);
    // std::thread p2(&SemiGlobalMatching::computeCensus,this,1);
    // if(p1.joinable())
    //    p1.join();
    // if(p2.joinable())
    //    p2.join();
 
    // 计算匹配代价

    computeCost();

    std::thread t1(&sgm_util::Util::computeAggregationHorizontal,this, cost_aggrs_[0],1);
    std::thread t2(&sgm_util::Util::computeAggregationHorizontal,this, cost_aggrs_[1],-1);
    std::thread t3(&sgm_util::Util::computeAggregationVertical,this,cost_aggrs_[2], 1);
    std::thread t4(sgm_util::Util::computeAggregationVertical,this,cost_aggrs_[3], -1);
    if(t1.joinable())
       t1.join();
    if(t2.joinable())
       t2.join();
    if(t3.joinable())
       t3.join();
    if(t4.joinable())
       t4.join();

    // sgm_util::Util::computeAggregationHorizontal(this, cost_aggrs_[0],1);
    // sgm_util::Util::computeAggregationHorizontal(this, cost_aggrs_[1],-1);
    // sgm_util::Util::computeAggregationVertical(this,cost_aggrs_[2], 1);
    // sgm_util::Util::computeAggregationVertical(this,cost_aggrs_[3], -1);

    int32_t size = height_ * width_ * option_.get_disparity_range();

    {
    auto start = std::chrono::system_clock::now();
    uint8_t *ptr1 = cost_aggrs_[0];
    uint8_t *ptr2 = cost_aggrs_[1];
    uint8_t *ptr3 = cost_aggrs_[2];
    uint8_t *ptr4 = cost_aggrs_[3];
    int32_t i = 0u;
    for (; i + 16 < size; i += 16) {
        // 加载四个数组的 16 个元素并零扩展为 uint16_t 向量
        __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr1 + i));
        __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr2 + i));
        __m128i vc = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr3 + i));
        __m128i vd = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr4 + i));

        __m256i s1 = _mm256_cvtepi8_epi16(va);
        __m256i s2 = _mm256_cvtepi8_epi16(vb);
        __m256i s3 = _mm256_cvtepi8_epi16(vc);
        __m256i s4 = _mm256_cvtepi8_epi16(vd);

        __m256i sum1 = _mm256_add_epi16(s1,s2);
        __m256i sum2 = _mm256_add_epi16(s3,s4);
        __m256i total = _mm256_add_epi16(sum1,sum2);

        _mm256_storeu_si256(reinterpret_cast<__m256i *>(cost_aggr_ + i),total);
        // // 先计算 (a + b) 和 (c + d)
        // __m128i v_ab = _mm_add_epi16(va, vb);
        // __m128i v_cd = _mm_add_epi16(vc, vd);

        // // 再将 (a + b) 和 (c + d) 相加得到最终结果
        // __m128i v_result = _mm_add_epi16(v_ab, v_cd);

        // // 存储结果到 uint16_t 数组
        // _mm_storeu_si128(reinterpret_cast<__m128i*>(cost_aggr_ + i), v_result);
        // for (int j = 0; j < 8; ++j) {
            // const int num = j;
            // short value = _mm_extract_epi16(v_result, 0);
            // std::cout << "Element " << 0 << ": " << value << std::endl;
        // }
    }
    for(; i < size; i++)
    {
        cost_aggr_[i] = ptr1[i] + ptr2[i] + ptr3[i] + ptr4[i];
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout<<"Spend time "<<duration<<" ms"<<std::endl;
  }
  

    // uchar *ptr1 = cost_aggrs_[0];
    // uchar *ptr2 = cost_aggrs_[1];
    // uchar *ptr3 = cost_aggrs_[2];
    // uchar *ptr4 = cost_aggrs_[3];
    // for(int32_t i = 0u; i < size; i++)
    // {
    //     cost_aggr_[i] = ptr1[i] + ptr2[i] + ptr3[i] + ptr4[i];
    // }

    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout<<"Spend time "<<duration<<" ms"<<std::endl;
}


void SemiGlobalMatching::display()
{
    cv::Mat img;
    img.create(cv::Size(width_,height_),CV_8UC1);
    for(int32_t i = 0; i < height_ ;i++)
    {
        for(int32_t j = 0; j < width_ ;j++)
        {
            uint8_t min_cost = UINT8_MAX;
            uint8_t best_dist = 0u;
            int32_t offest = i * width_ * option_.get_disparity_range() + j * option_.get_disparity_range();
            for(uint8_t d = 0 ; d < option_.get_disparity_range();d++)
            {
                uint16_t cost_val = cost_aggr_[offest + d];
                if(cost_val < min_cost)
                {
                    min_cost = cost_val;
                    best_dist = d;
                }
            }
            img.at<uchar>(i,j) = best_dist;
        }
    }
    cv::imshow("Display",img);
    cv::waitKey(0);
}

void SemiGlobalMatching::initialize(const int width,const int height)
{
    width_ = width;
    height_ = height;

    // 分配内存
    left_census_  = new uint32_t[width_ * height_];
    right_census_ = new uint32_t[width_ * height_];
    cost_init_    = new uint8_t[height_ * width_ * option_.get_disparity_range()];
    cost_aggr_    = new uint16_t[height_ * width_ * option_.get_disparity_range()];

    cost_aggrs_.resize(4,nullptr);
    for(auto &cost_aggr:cost_aggrs_)
    {
        cost_aggr  = new uint8_t[height_ * width_ * option_.get_disparity_range()];
    }

    initialized_ = true;
}

void SemiGlobalMatching::releaseResource()
{
    RELEASE_RESOURCE(left_census_);
    RELEASE_RESOURCE(right_census_);
    RELEASE_RESOURCE(cost_init_);
    RELEASE_RESOURCE(cost_aggr_);
    for(auto &cost_aggr:cost_aggrs_)
    {
        RELEASE_RESOURCE(cost_aggr);
    }
}

void SemiGlobalMatching::computeCensus(const bool flag)
{
    uint32_t *census = nullptr;
    census  = flag ? left_census_ : right_census_;
    auto img = flag ? left_img_ : right_img_;

    // 进行滑动窗口大小为5x5的计算
    // TODO: 多重嵌套循环结构还是不太友好
    // for(int row = 2; row < height_ - 2; row++)
    // {
    //     for(int col = 2; col < width_ - 2; col++)
    //     {
    //         uchar center_val = img->at<uchar>(row,col);
    //         uint32_t census_val = 0u;
    //         for(int i = -2 ; i < 3;i++)
    //         {
    //             for(int j = -2; j < 3;j++)
    //             {
    //                 census_val <<=1;
    //                 uchar  pixel_val =  img->at<uchar>(row + i,col + j);
    //                 if(pixel_val < center_val)
    //                    census_val++;
    //             }
    //         }
    //         census[row * width_ + col] = census_val;
    //     }
    // }

    static auto func = [this](uint32_t *census,std::shared_ptr<cv::Mat> img,int32_t begin,int32_t end){
        for(int row = begin ; row < end; row++)
        {
            for(int col = 2; col < width_ - 2; col++)
            {
                uchar center_val = img->at<uchar>(row,col);
                uint32_t census_val = 0u;
                for(int i = -2 ; i < 3;i++)
                {
                    // TODO:可能会有跟census一样能表示相对关系的算子,虽然会增加内存负担,但是提升了效率
                    for(int j = -2; j < 3;j++)
                    {
                        census_val <<=1;
                        uchar  pixel_val =  img->at<uchar>(row + i,col + j);
                        if(pixel_val < center_val)
                        census_val++;
                    }
                }
            census[row * width_ + col] = census_val;            
            }
        }
    };

    if(option_.isAccel())
    {
        constexpr int threshold = 200;
        int num = height_ / threshold;
        if( num <= 1 ){
            func(census,img,2,height_-2);
            return ;
        }
        std::vector<std::thread> threads;

        for( int i = 1; i < num - 1;i++)
        {
            threads.push_back(std::thread(func,census,img,i* threshold,(i+1)*threshold));
        }
        threads.push_back(std::thread(func,census,img,2,threshold));
        threads.push_back(std::thread(func,census,img,(num - 1) * threshold,height_-2));
        for(std::thread &t:threads)
        {
            if(t.joinable())
               t.join();
        }
    } else 
      func(census,img,2,height_-2);
}

void SemiGlobalMatching::computeCost()
{
    // TODO:暂时先考虑使用多线程并发处理,后面在想想怎么是实现向量化
    //      1.目前可以向量化的部分

    static auto func = [this](int32_t begin,int32_t end)
    {
        const int min_disparity   = option_.get_min_disparity();
        const int max_disparity   = option_.get_max_disparity();
        const int disparity_range = option_.get_disparity_range();
        for(int i = begin; i < end; i++)
        {
            for(int j = 0; j < width_; j++)
            {
                int offest = i * width_ * disparity_range + j * disparity_range;
                for(int d  = min_disparity; d < max_disparity; d++)
                {
                    const uint32_t& lcensus = left_census_[i * width_ + j];
                    uint8_t& cost = cost_init_[offest + d - min_disparity ];
                    if(j - d < 0 || j - d  >= width_)
                    {
                        cost = UINT8_MAX / 2;
                        continue;
                    }
                    
                    const uint32_t& rcensus = right_census_[i * width_ + j - d ];

                    cost  = sgm_util::Util::Hamming(lcensus,rcensus);
                }
            }
        }    
    };

    // {
    // auto start = std::chrono::system_clock::now();
    // const int min_disparity   = option_.get_min_disparity();
    // const int max_disparity   = option_.get_max_disparity();
    // const int disparity_range = option_.get_disparity_range();
    // for(int i = 0; i < height_; i++)
    // {
    //     for(int j = 0; j < width_; j++)
    //     {
    //         int offest = i * width_ * disparity_range + j * disparity_range;
    //         for(int d  = min_disparity; d < max_disparity; d++)
    //         {
    //             const uint32_t& lcensus = left_census_[i * width_ + j];
    //             uint8_t& cost = cost_init_[offest + d - min_disparity ];
    //             if(j - d < 0 || j - d  >= width_)
    //             {
    //                 cost = UINT8_MAX / 2;
    //                 continue;
    //             }
                
    //             const uint32_t& rcensus = right_census_[i * width_ + j - d ];

    //             cost  = sgm_util::Util::Hamming(lcensus,rcensus);
    //         }
    //     }
    // }
    // auto end = std::chrono::system_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // std::cout<<"Spend time "<<duration<<" ms"<<std::endl;
    // }

    {
    auto start = std::chrono::system_clock::now();
    constexpr uint32_t threshold = 100;
    int num = height_ / threshold;
    if(num <= 1)
       func(0,height_);
    else 
    {
        std::vector<std::thread> threads;
        for( int i = 0; i < num - 1; i++)
        {
            threads.push_back(std::thread(func,i * threshold, (i + 1)*threshold));
        }
        threads.push_back(std::thread(func,(num - 1) * threshold, height_));
        std::cout<<"thread number "<<static_cast<int>(threads.size())<<std::endl;
        for(std::thread &t:threads)
        {
            if(t.joinable())
               t.join();
        }
    }
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout<<"Spend time "<<duration<<" ms"<<std::endl;
    }
}

}