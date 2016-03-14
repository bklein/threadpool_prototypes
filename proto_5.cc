#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

#include <tbb/concurrent_queue.h>

#include <boost/threadpool.hpp>

class WorkQueue {
 public:
  explicit WorkQueue(size_t num_threads)
    : running_(true),
      jobs_(),
      thread_group_() {
    for (size_t i = 0; i < num_threads; ++i) {
      thread_group_.create_thread(boost::bind(&WorkQueue::run, this));
    }
  }

  void enqueue(const std::function<void(void)>& f) {
    jobs_.push(f);
  }

  ~WorkQueue() {
    running_ = false;
    jobs_.push(std::function<void(void)>([](){}));
  }

 private:
  void run() {
    while (true) {
      std::function<void(void)> job;
      if (jobs_.try_pop(job)) {
        std::cout << "doing job" << std::endl;
        job();
      } else {
        std::cout << "no job" << std::endl;
      }
      //else if (!running_ && jobs_.empty()) break;
    }
  }

  std::atomic<bool> running_;
  boost::thread_group thread_group_;
  tbb::concurrent_queue<std::function<void(void)>> jobs_;

};


int main() {

  {

    WorkQueue queue(2);
    queue.enqueue([](){
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "done 1" << std::endl;
      });

    queue.enqueue([](){
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "done 2" << std::endl;
      });

    std::cout << "done enqueuing" << std::endl;

  }
  std::cout << "done" << std::endl;

  return 0;
}
