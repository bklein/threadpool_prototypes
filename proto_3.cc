#include <limits>
#include <thread>
#include <iostream>

#include <boost/threadpool.hpp>

static
int doWork(int max) {
  int n = 0;
  for (int i = 0; i < max; ++i) {
    if (i % 2 == 0)
      ++n;
    else
      --n;
  }
  return n;
  std::cout << "thread: " << std::this_thread::get_id() << std::endl;
}

int main(void) {

  boost::threadpool::pool pool(1);

  std::cout << "scheduling" << std::endl;
  for (int i = 0; i < 1024; ++i) {
    pool.schedule(boost::bind(&doWork, std::numeric_limits<int8_t>::max()));
  }
  std::cout << "done" << std::endl;

  return 0;
}
