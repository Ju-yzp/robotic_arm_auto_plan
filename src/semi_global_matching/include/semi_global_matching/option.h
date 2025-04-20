#ifndef  SEMI_GOLBAL_MATCHING_OPTION_H_
#define  SEMI_GOLBAL_MATCHING_OPTION_H_

#include <cassert>
#include <stdexcept>

namespace semi_global_matching {

class MatchOption
{
    public:
    explicit MatchOption(int min_disparity, int max_disparity , int factor,int penalty_factor,
    bool multi_thread_accel = true):
    min_disparity_(min_disparity),
    max_disparity_(max_disparity),
    factor_(factor),
    penalty_factor_(penalty_factor),
    multi_thread_accel_(multi_thread_accel)
    {
        if(max_disparity <= min_disparity || min_disparity < 0)
           throw std::runtime_error("Invaild disparity value");
        disparity_range_ = max_disparity_ - min_disparity_;
    }

    bool isAccel(){ return multi_thread_accel_;};

    const int get_min_disparity(){ return min_disparity_; }

    const int get_max_disparity(){ return max_disparity_; }

    const int get_disparity_range(){ return disparity_range_; }

    int get_factor(){ return factor_;};

    int get_penalty_factor(){ return penalty_factor_; }

    void set_min_disparity_range(int min_disparity){ min_disparity_ = min_disparity;}

    void set_max_disparity_range(int max_disparity){ max_disparity_ = max_disparity;}

    private:
    // 
    int min_disparity_;
    int max_disparity_;
    int disparity_range_;

    // 
    int factor_;
    int penalty_factor_;
    
    //
    bool multi_thread_accel_;
};

}

#endif