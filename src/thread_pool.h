#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t num_workers);
  ~ThreadPool();

  // Add a task to the queue
  void enqueue(std::function<void()> task);

  // Gracefully shutdown the thread pool
  void shutdown();

  // Check if thread pool is running
  bool is_running() const;

 private:
  // Task queue
  std::queue<std::function<void()>> task_queue_;

  // Synchronization primitives
  std::mutex queue_mutex_;
  std::condition_variable queue_cv_;

  // Worker threads
  std::vector<std::thread> workers_;

  // Shutdown flag
  bool shutdown_requested_;

  // Worker thread function
  void worker_loop();
};
