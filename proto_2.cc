#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include <tbb/concurrent_queue.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

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
}

class ThreadPool {
 public:
  using Work = std::function<void(void)>;
  ThreadPool(int num_threads)
    : io_service_(),
      work_(io_service_),
      thread_group_() {
    for (int i = 0; i < num_threads; ++i) {
      thread_group_.create_thread(boost::bind(&ThreadPool::run, this));
    }
  }

  void run() {
    //while (io_service_.run_one());
    io_service_.run();
  }

  ~ThreadPool() {
    io_service_.stop();
    thread_group_.join_all();
  }

  void addWork(const Work& work) {
    io_service_.post(work);
    //io_service_.wrap(work);
  }

 private:
  boost::asio::io_service io_service_;
  boost::asio::io_service::work work_;
  boost::thread_group thread_group_;
};

std::mutex output_mutex;
void log() {
  std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << "thread: " << std::this_thread::get_id() << std::endl;
}

int main() {
  const size_t kNumTasks = 100000000;  // 100 million

  std::vector<int> answers(kNumTasks, 0);

  {
    ThreadPool thread_pool(4);

    const size_t start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    for (int i = 0; i < kNumTasks; ++i) {
      ThreadPool::Work work =
        [&answers, i] (void) -> void {
          //log();
          answers[i] = doWork(i);
        };
      thread_pool.addWork(work);
      //if (i % 100000 == 0) {
      //  const size_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      //  const float elapsed_secs = (cur_time - start_time) * 0.001f;
      //  std::cout << "i: " << i << "/" << kNumTasks << " " << (i * 100.0f / kNumTasks) << "%, secs elapsed: " << elapsed_secs << std::endl;
      //}
    }
    {
      std::lock_guard<std::mutex> lock(output_mutex);
      std::cout << "done adding work" << std::endl;
    }
  }

  {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "done" << std::endl;
  }

  return 0;
}
