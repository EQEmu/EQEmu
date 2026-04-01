/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
