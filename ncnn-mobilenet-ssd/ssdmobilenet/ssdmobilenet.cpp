// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>
#include <unistd.h>
#include "net.h"

#include <iostream>
#include <ctime>

class Timer
{
public:
    Timer() { clock_gettime(CLOCK_REALTIME, &beg_); }

    double elapsed() {
        clock_gettime(CLOCK_REALTIME, &end_);
        return end_.tv_sec - beg_.tv_sec +
            (end_.tv_nsec - beg_.tv_nsec) / 1000000000.;
    }

    void reset() { clock_gettime(CLOCK_REALTIME, &beg_); }

private:
    timespec beg_, end_;
};

static int detect_mobilenet(const cv::Mat& bgr, std::vector<float>& cls_scores)
{
    ncnn::Net mobilenet;
    mobilenet.load_param("ssdmobilenet.param");
    mobilenet.load_model("ssdmobilenet.bin");

    Timer tmr;
    double t = tmr.elapsed();
    std::cout << t << std::endl;


    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, bgr.cols, bgr.rows, 300, 300);

    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
    in.substract_mean_div(mean_vals, 1.0/127.5f);

    // ncnn::Extractor ex = mobilenet.create_extractor();
    // ex.set_light_mode(true);
    // ex.set_num_threads(4);

    // ex.input("data", in);

    ncnn::Mat out;
    //ex.set_light_mode(true);

    for(int i=0;i<100;i++)
    {
        tmr.reset();
        ncnn::Extractor ex = mobilenet.create_extractor();
        ex.set_num_threads(4);
        ex.input("data", in);
        ex.extract("detection_out",out);

        t = tmr.elapsed();
        std::cout << "one round ssd cost" << std::endl;
        std::cout << t << std::endl;
        //ex.extract("conv6/dw_conv6/dw/relu", out);    

    }

    printf("%d %d %d\n", out.w, out.h, out.c);

    for (int ih=0;ih<out.h;ih++)
    {
        for(int iw=0;iw<out.w;iw++)
        {
                printf("%f ", out[iw+ih*out.w]);                 
        }
        printf("\n");
    }

/*
    cls_scores.resize(out.c);
    for (int j=0; j<out.c; j++)
    {
        const float* prob = out.data + out.cstep * j;
        cls_scores[j] = prob[0];
    }
*/
    return 0;
}

static int print_topk(const std::vector<float>& cls_scores, int topk)
{
    // partial sort topk with index
    int size = cls_scores.size();
    std::vector< std::pair<float, int> > vec;
    vec.resize(size);
    for (int i=0; i<size; i++)
    {
        vec[i] = std::make_pair(cls_scores[i], i);
    }

    std::partial_sort(vec.begin(), vec.begin() + topk, vec.end(),
                      std::greater< std::pair<float, int> >());

    // print topk and score
    for (int i=0; i<topk; i++)
    {
        float score = vec[i].first;
        int index = vec[i].second;
        fprintf(stderr, "%d = %f\n", index, score);
    }

    return 0;
}

int main(int argc, char** argv)
{
    const char* imagepath = argv[1];

    cv::Mat m = cv::imread(imagepath, 1);//CV_LOAD_IMAGE_COLOR);
    if (m.empty())
    {
        fprintf(stderr, "cv::imread %s failed\n", imagepath);
        return -1;
    }

    std::vector<float> cls_scores;
    detect_mobilenet(m, cls_scores);

    //print_topk(cls_scores, 3);

    return 0;
}

