#include "thread_pool.hpp"

#include <chrono>
#include <iostream>
#include <thread>

static
void diceRoll() {
  const auto t1 =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  bool heads = false;
  while (true) {
    const auto t2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if (t2 - t1 > 33)
      break;
    else
      heads = !heads;
  }
}

int main() {
  const size_t kNumTasks = 15000;

  const size_t kNThreads = 4;
  auto thread_pool = std::make_shared<ThreadPool>(kNThreads);

  for (int i = 0; i < kNumTasks; ++i) {
    ThreadPool::Work work = std::bind(&diceRoll);
    thread_pool->addWork(work);
  }

  std::cout << "done adding work" << std::endl;

  while (true) {
    const auto added = thread_pool->jobs_added();
    const auto done = thread_pool->jobs_done();
    std::cout << "jobs: " << added << "/" << done << std::endl;
    if (added == done)
      break;
    else
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  thread_pool.reset();

  return 0;
}
