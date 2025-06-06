#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class BRThreadPool {
public:
	BRThreadPool();
	void init(size_t);
	template <class F, class... Args>
	auto enqueue(F &&f, Args &&...args)
			-> std::future<typename std::invoke_result_t<F,Args... >>;
	~BRThreadPool();

	size_t size();

private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> workers;
	// the task queue
	std::queue<std::function<void()>> tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
};

inline BRThreadPool::BRThreadPool() :
		stop(false) {}

// the constructor just launches some amount of workers
inline void BRThreadPool::init(size_t threads) {
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back(
				[this] {
					for (;;) {
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->queue_mutex);
							this->condition.wait(lock,
									[this] { return this->stop || !this->tasks.empty(); });
							if (this->stop && this->tasks.empty())
								return;
							task = std::move(this->tasks.front());
							this->tasks.pop();
						}

						task();
					}
				});
}

// add new work item to the pool
template <class F, class... Args>
auto BRThreadPool::enqueue(F &&f, Args &&...args)
		-> std::future<typename std::invoke_result_t<F, Args...>> {
	using return_type = typename std::invoke_result_t<F, Args...>;

	auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// don't allow enqueueing after stopping the pool
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace([task]() { (*task)(); });
	}
	condition.notify_one();
	return res;
}

// the destructor joins all threads
inline BRThreadPool::~BRThreadPool() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();
}

inline size_t BRThreadPool::size() {
	return this->workers.size();
}

#endif
