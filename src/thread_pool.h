#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <stdexcept>

class ThreadPool {
public:
    // Create a pool with the specified number of worker threads.
    ThreadPool(size_t threads);
    ~ThreadPool();

    // Enqueue a task for execution. Returns a future holding the task's return value.
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    // Gracefully shutdown the thread pool.
    void shutdown();

private:
    // Vector storing worker threads.
    std::vector<std::thread> workers;
    // Task queue.
    std::queue<std::function<void()>> tasks;

    // Synchronization.
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop.load())
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

#endif // THREAD_POOL_H
