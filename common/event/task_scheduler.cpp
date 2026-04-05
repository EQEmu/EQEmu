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

#include "task_scheduler.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace EQ::Event {

static constexpr int DefaultThreadCount = 4;

struct TaskScheduler::SchedulerData
{
	std::atomic_bool running{ false };
	std::vector<std::thread> threads;
	std::mutex lock;
	std::condition_variable cv;
	std::queue<std::function<void()>> tasks;
};

TaskScheduler::TaskScheduler()
	: m_data(std::make_unique<SchedulerData>())
{
	Start(DefaultThreadCount);
}

TaskScheduler::TaskScheduler(size_t threads)
{
	Start(threads);
}

TaskScheduler::~TaskScheduler()
{
	Stop();
}

void TaskScheduler::Start(size_t threads)
{
	if (m_data->running.exchange(true))
		return;

	m_data->threads.reserve(threads);

	for (size_t i = 0; i < threads; ++i)
	{
		m_data->threads.emplace_back(
			[this]{ ProcessWork(); });
	}
}

void TaskScheduler::Stop()
{
	if (!m_data->running.exchange(false))
		return;

	m_data->cv.notify_all();

	for (auto& t : m_data->threads)
	{
		t.join();
	}

	m_data->threads.clear();
}

void TaskScheduler::ProcessWork()
{
	for (;;)
	{
		std::function<void()> work;

		{
			std::unique_lock lock(m_data->lock);

			m_data->cv.wait(lock,
				[this]
			{
				return !m_data->running || !m_data->tasks.empty();
			});

			if (!m_data->running)
			{
				return;
			}

			work = std::move(m_data->tasks.front());
			m_data->tasks.pop();
		}

		work();
	}
}

void TaskScheduler::AddTask(std::function<void()>&& task)
{
	if (!m_data->running)
	{
		throw std::runtime_error("Enqueue on stopped scheduler.");
	}

	{
		std::scoped_lock lock(m_data->lock);
		m_data->tasks.push(std::move(task));
	}
	m_data->cv.notify_one();
}

} // namespace EQ::Event
