#include "ThreadPool.h"

#include "Profiler.h"

#include <algorithm>
#include <string>

namespace gfx1993 {

static size_t defaultThreadCount() {
  unsigned int n = std::thread::hardware_concurrency();
  return n == 0 ? 1 : static_cast<size_t>(n);
}

ThreadPool::ThreadPool() : ThreadPool(defaultThreadCount()) {}

ThreadPool::ThreadPool(size_t threadCount) {
  if (threadCount == 0) {
    threadCount = 1;
  }
  ranges.resize(threadCount);
  workers.reserve(threadCount);
  for (size_t i = 0; i < threadCount; ++i) {
    workers.emplace_back(&ThreadPool::workerLoop, this, i);
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    stop = true;
  }
  startCv.notify_all();
  for (auto &worker : workers) {
    worker.join();
  }
}

void ThreadPool::workerLoop(size_t workerIndex) {
  thread_local std::string threadName =
      "gfx1993-worker-" + std::to_string(workerIndex);
  GFX1993_SET_THREAD_NAME(threadName.c_str());

  size_t seenGeneration = 0;
  for (;;) {
    std::unique_lock<std::mutex> lock(mutex);
    startCv.wait(lock, [&] { return stop || generation != seenGeneration; });
    if (stop) {
      return;
    }
    seenGeneration = generation;
    const auto *fn = currentFn;
    const auto range = ranges[workerIndex];
    lock.unlock();

    if (fn && range.second > range.first) {
      GFX1993_ZONE_N("ThreadPool::worker");
      (*fn)(workerIndex, range.first, range.second);
    }

    lock.lock();
    --pending;
    if (pending == 0) {
      lock.unlock();
      doneCv.notify_one();
    }
  }
}

void ThreadPool::parallelFor(
    size_t count,
    const std::function<void(size_t worker, size_t begin, size_t end)> &fn) {
  if (count == 0) {
    return;
  }

  const size_t threadCount = workers.size();
  const size_t chunk = (count + threadCount - 1) / threadCount;

  std::unique_lock<std::mutex> lock(mutex);
  for (size_t w = 0; w < threadCount; ++w) {
    const size_t begin = std::min(w * chunk, count);
    const size_t end = std::min(begin + chunk, count);
    ranges[w] = {begin, end};
  }
  currentFn = &fn;
  pending = threadCount;
  ++generation;
  lock.unlock();
  startCv.notify_all();

  lock.lock();
  doneCv.wait(lock, [&] { return pending == 0; });
  currentFn = nullptr;
}

} // namespace gfx1993
