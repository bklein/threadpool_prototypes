#ifndef PROTOTYPE_THREAD_POOL_HPP_
#define PROTOTYPE_THREAD_POOL_HPP_

#include <functional>
#include <memory>

class ThreadPool {
 public:
  using Work = std::function<void(void)>;

  explicit ThreadPool(size_t num_threads);

  void addWork(const Work& work);

  size_t jobs_added() const;
  size_t jobs_done() const;

  ~ThreadPool();

 private:
  class Impl;
  std::unique_ptr<Impl> impl;
};


#endif  // PROTOTYPE_THREAD_POOL_HPP_
