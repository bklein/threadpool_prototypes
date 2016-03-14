#include <atomic>
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
void diceRoll() {
  const auto t1 =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  bool heads = false;
  while (true) {
    const auto t2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if (t2 - t1 > 100)
      break;
    else
      heads = !heads;
  }
}

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

std::mutex output_mutex;
void log() {
  std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << "thread: " << std::this_thread::get_id() << std::endl;
}

void longLog() {
  {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "starting task on thread: " << std::this_thread::get_id() << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
  {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "ending task on thread: " << std::this_thread::get_id() << std::endl;
  }

}


class ThreadPool {
 public:
  using Work = std::function<void(void)>;
  ThreadPool(int num_threads)
    : jobs_added_(0),
      jobs_done_(0),
      io_service_(num_threads),
      work_(new boost::asio::io_service::work(io_service_)),
      thread_group_() {
    for (int i = 0; i < num_threads; ++i) {
      thread_group_.create_thread(boost::bind(&ThreadPool::run, this));
    }
  }

  void run() {
    io_service_.run();
  }

  ~ThreadPool() {
    work_.reset();
    thread_group_.join_all();
  }

  void addWork(const Work& work) {
    auto reporter =
      [this, work] (void) -> void {
        work();
        ++jobs_done_;
        log();
      };
    io_service_.post(reporter);
    ++jobs_added_;
  }

 private:
  std::atomic<size_t> jobs_added_;
  std::atomic<size_t> jobs_done_;

  boost::asio::io_service io_service_;
  std::unique_ptr<boost::asio::io_service::work> work_;
  boost::thread_group thread_group_;

  void log() const {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "jobs: " << jobs_added_ << " / " << jobs_done_ << std::endl;
  }
};

int main() {
  const size_t kNumTasks = 15000;

  //std::vector<int> answers(kNumTasks, 0);

  {
    ThreadPool thread_pool(4);

    const size_t start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    for (int i = 0; i < kNumTasks; ++i) {
      ThreadPool::Work work = boost::bind(&diceRoll);
      //ThreadPool::Work work = boost::bind(&longLog);
        //[&answers, i] (void) -> void {
        //  //log();
        //  answers[i] = doWork(i);
        //};
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
