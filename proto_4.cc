#include <vector>
#include <cmath>
#include <thread>
#include <iostream>
#include <limits>

#include <tbb/pipeline.h>

float RootMeanSquare( float* first, float* last ) {
  float sum=0;
  tbb::parallel_pipeline( /*max_number_of_live_token=*/16,
      tbb::make_filter<void,float*>(
        tbb::filter::serial,
        [&](tbb::flow_control& fc)-> float*{
          if( first<last ) {
            return first++;
          } else {
            fc.stop();
            return NULL;
          }
        }) &
      tbb::make_filter<float*,float>(
        tbb::filter::parallel,
        [](float* p){
          return (*p)*(*p);
        }) &
      tbb::make_filter<float,void>(
        tbb::filter::serial,
        [&](float x) {
          sum+=x;
        })
      );
  return std::sqrt(sum);
}

int main(void) {

  std::vector<float> n(100000, 2.0f);
  RootMeanSquare(&n.front(), &n.back());

  return 0;
}
