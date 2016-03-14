#include "thread_pool.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

class ThreadPool::Impl {
 public:
  using Work = ThreadPool::Work;
  explicit Impl(size_t num_threads)
    : jobs_added_(0),
      jobs_done_(0),
      io_service_(num_threads),
      work_(new boost::asio::io_service::work(io_service_)),
      thread_group_() {
    for (size_t i = 0; i < num_threads; ++i) {
      thread_group_.create_thread(boost::bind(&Impl::run, this));
    }
  }

  size_t jobs_added() const {
    return jobs_added_;
  }

  size_t jobs_done() const {
    return jobs_done_;
  }

  void addWork(const Work& work) {
    auto reporter =
      [this, work] (void) -> void {
        work();
        ++jobs_done_;
      };
    io_service_.post(reporter);
    ++jobs_added_;
  }

  ~Impl() {
    work_.reset();
    thread_group_.join_all();
  }

 private:
  std::atomic<size_t> jobs_added_;
  std::atomic<size_t> jobs_done_;

  boost::asio::io_service io_service_;
  std::unique_ptr<boost::asio::io_service::work> work_;
  boost::thread_group thread_group_;

  void run() {
    io_service_.run();
  }

};

ThreadPool::ThreadPool(size_t num_threads)
  : impl(new Impl(num_threads)) {
}

ThreadPool::~ThreadPool() {
  impl.reset();
}

size_t ThreadPool::jobs_added() const {
  return impl->jobs_added();
}

size_t ThreadPool::jobs_done() const {
  return impl->jobs_done();
}

void ThreadPool::addWork(const Work& work) {
  impl->addWork(work);
}
