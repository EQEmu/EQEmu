#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace EQ
{
	namespace Event
	{
		class TaskScheduler
		{
		public:
			static const int DefaultThreadCount = 4;
		
			TaskScheduler() : _running(false)
			{
				Start(DefaultThreadCount);
			}

			TaskScheduler(size_t threads) : _running(false)
			{
				Start(threads);
			}
			
			~TaskScheduler() {
				Stop();
			}

			void Start(size_t threads) {
				if (true == _running) {
					return;
				}

				_running = true;

				for (size_t i = 0; i < threads; ++i) {
					_threads.emplace_back(std::thread(std::bind(&TaskScheduler::ProcessWork, this)));
				}
			}
			
			void Stop() {
				if (false == _running) {
					return;
				}

				{
					std::unique_lock<std::mutex> lock(_lock);
					_running = false;
				}

				_cv.notify_all();

				for (auto &t : _threads) {
					t.join();
				}
			}
			
			template<typename Fn, typename... Args>
			auto Enqueue(Fn&& fn, Args&&... args) -> std::future<typename std::invoke_result<Fn, Args...>::type> {
				using return_type = typename std::invoke_result<Fn, Args...>::type;
			
				auto task = std::make_shared<std::packaged_task<return_type()>>(
					std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...)
					);
			
				std::future<return_type> res = task->get_future();
				{
					std::unique_lock<std::mutex> lock(_lock);
			
					if (false == _running) {
						throw std::runtime_error("Enqueue on stopped scheduler.");
					}
			
					_tasks.emplace([task]() { (*task)(); });
				}
			
				_cv.notify_one();
				return res;
			}
			
			private:
			void ProcessWork() {
				for (;;) {
					std::function<void()> work;

					{
						std::unique_lock<std::mutex> lock(_lock);
						_cv.wait(lock, [this] { return !_running || !_tasks.empty(); });

						if (false == _running) {
							return;
						}

						work = std::move(_tasks.front());
						_tasks.pop();
					}

					work();
				}
				
			}

			bool _running = true;
			std::vector<std::thread> _threads;
			std::mutex _lock;
			std::condition_variable _cv;
			std::queue<std::function<void()>> _tasks;
		};
	}
}
