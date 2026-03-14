#include "thread_pool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t num_workers) : shutdown_requested_(false) {
  // Create num_workers threads
  for (size_t i = 0; i < num_workers; ++i) {
    workers_.emplace_back(&ThreadPool::worker_loop, this);
  }

  std::cout << "ThreadPool created with " << num_workers << " workers"
            << std::endl;
}

ThreadPool::~ThreadPool() {
  shutdown();
}

void ThreadPool::enqueue(std::function<void()> task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (shutdown_requested_) {
      std::cerr << "Cannot enqueue task: thread pool is shutting down"
                << std::endl;
      return;
    }
    task_queue_.push(task);
  }
  // Notify one waiting worker
  queue_cv_.notify_one();
}

void ThreadPool::shutdown() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    shutdown_requested_ = true;
  }
  // Wake all workers so they can exit
  queue_cv_.notify_all();

  // Join all worker threads
  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  std::cout << "ThreadPool shutdown complete" << std::endl;
}

bool ThreadPool::is_running() const {
  return !shutdown_requested_;
}

void ThreadPool::worker_loop() {
  while (true) {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // Wait until there's a task or shutdown is requested
    queue_cv_.wait(lock, [this]() {
      return !task_queue_.empty() || shutdown_requested_;
    });

    // If shutdown requested and queue is empty, exit
    if (shutdown_requested_ && task_queue_.empty()) {
      break;
    }

    // Get a task from the queue
    if (!task_queue_.empty()) {
      std::function<void()> task = task_queue_.front();
      task_queue_.pop();
      lock.unlock();  // Unlock before executing task

      // Execute the task
      task();
    }
  }
}
