#ifndef GFX1993_THREADPOOL_INCLUDED
#define GFX1993_THREADPOOL_INCLUDED

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace gfx1993 {

// A fixed-size, persistent pool of worker threads used to fan out
// "parallel for" style work without OpenMP. Threads are created once in the
// constructor and parked between calls, so repeated parallelFor() calls (e.g.
// once per frame) don't pay thread-creation cost.
//
// Unlike a naive OpenMP "critical section per item" pattern, callers should
// give each worker its own output buffer (indexed by the `worker` argument)
// and merge the buffers themselves after parallelFor() returns -- this keeps
// workers lock-free while they do the actual work.
class ThreadPool {
public:
  // Sizes the pool to the hardware's reported concurrency (falling back to 1
  // if that can't be determined).
  ThreadPool();
  explicit ThreadPool(size_t threadCount);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  size_t getThreadCount() const { return workers.size(); }

  // Splits [0, count) into getThreadCount() contiguous chunks and invokes
  // `fn(worker, begin, end)` for each chunk on a worker thread, blocking
  // until every chunk has finished. Safe to call with count == 0 (no-op).
  void parallelFor(size_t count,
                    const std::function<void(size_t worker, size_t begin, size_t end)> &fn);

private:
  void workerLoop(size_t workerIndex);

  std::vector<std::thread> workers;
  std::vector<std::pair<size_t, size_t>> ranges;

  std::mutex mutex;
  std::condition_variable startCv;
  std::condition_variable doneCv;

  const std::function<void(size_t, size_t, size_t)> *currentFn = nullptr;
  size_t generation = 0;
  size_t pending = 0;
  bool stop = false;
};

} // namespace gfx1993

#endif // GFX1993_THREADPOOL_INCLUDED
