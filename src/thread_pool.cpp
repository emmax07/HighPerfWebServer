#include "server/thread_pool.hpp"
#include <iostream>

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads == 0) num_threads = 4; // Fallback to 4 threads

    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this]() {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    
                    // Wait until there is a task or the thread pool is stopping
                    this->cv_.wait(lock, [this]() {
                        return this->stop_.load() || !this->tasks_.empty();
                    });

                    if (this->stop_.load() && this->tasks_.empty()) {
                        return;
                    }

                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }

                // Execute task outside the lock
                task();
            }
        });
    }
    std::cout << "[ThreadPool] Spawned " << workers_.size() << " worker threads." << std::endl;
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_.load()) {
            return;
        }
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::stop() {
    bool expected = false;
    if (stop_.compare_exchange_strong(expected, true)) {
        cv_.notify_all();
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "[ThreadPool] All worker threads shut down safely." << std::endl;
    }
}

ThreadPool::~ThreadPool() {
    stop();
}