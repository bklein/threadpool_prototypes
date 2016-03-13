#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

class Printer {
 public:
  Printer(boost::asio::io_service& io_service)
    : strand_(io_service),
      timer_1_(io_service, boost::posix_time::seconds(1)),
      timer_2_(io_service, boost::posix_time::seconds(1)),
      count_(0) {
    timer_1_.async_wait(strand_.wrap(boost::bind(&Printer::print1, this)));
    timer_2_.async_wait(strand_.wrap(boost::bind(&Printer::print2, this)));
  }

  ~Printer() {
    std::cout << "final count: " << count_ << std::endl;
  }

  void print1() {
    if (count_ < 10) {
      std::cout << "timer 1: " << count_ << std::endl;
      ++count_;
      timer_1_.expires_at(timer_1_.expires_at() + boost::posix_time::seconds(1));
      timer_1_.async_wait(strand_.wrap(boost::bind(&Printer::print1, this)));
    }
  }

  void print2() {
    if (count_ < 10) {
      std::cout << "timer 2: " << count_ << std::endl;
      ++count_;
      timer_1_.expires_at(timer_2_.expires_at() + boost::posix_time::seconds(1));
      timer_1_.async_wait(strand_.wrap(boost::bind(&Printer::print2, this)));
    }
  }

 private:
  boost::asio::io_service::strand strand_;
  boost::asio::deadline_timer timer_1_;
  boost::asio::deadline_timer timer_2_;
  int count_;

};

int main(void) {
  boost::asio::io_service io_service;

  std::cout << "begin" << std::endl;

  Printer p(io_service);

  //boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
  //io_service.run();
  std::size_t runs = 0;
  while (true) {
    const std::size_t num = io_service.run_one();
    runs += num;
    std::cout << "ran: " << num << ", total: " << runs << std::endl;
    if (num == 0) break;
  }
  //t.join();

  std::cout << "shutdown" << std::endl;

  return 0;
}
