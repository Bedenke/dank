#pragma once
namespace dank {

class Runnable {
public:
  virtual void run() = 0;
  virtual ~Runnable() = 0;
};

class ThreadData {};

class Thread {
public:
  ThreadData *data;
  Runnable *runnable;
  bool start(Runnable *runnable);
  void join();
};

} // namespace dank
