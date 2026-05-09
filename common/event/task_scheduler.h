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

#include <functional>
#include <future>
#include <memory>

namespace EQ::Event {

class TaskScheduler
{
public:
	TaskScheduler();
	TaskScheduler(size_t threads);
			
	~TaskScheduler();

	void Start(size_t threads);
	void Stop();

	template <typename Fn, typename... Args>
	auto Enqueue(Fn&& fn, Args&&... args) -> std::future<typename std::invoke_result<Fn, Args...>::type>
	{
		using return_type = typename std::invoke_result<Fn, Args...>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			[fn = std::forward<Fn>(fn), ...args = std::forward<Args>(args)]() mutable
			{
				return fn(std::forward<Args>(args)...);
			}
		);
			
		AddTask([task] { (*task)(); });
		return task->get_future();
	}

private:
	void AddTask(std::function<void()>&& task);
	void ProcessWork();

	struct SchedulerData;
	std::unique_ptr<SchedulerData> m_data;
};

} // namespace EQ::Event
