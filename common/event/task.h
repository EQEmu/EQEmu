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

#include "event_loop.h"

#include <any>
#include <exception>
#include <functional>

namespace EQ {
	class Task
	{
	public:
		typedef std::function<void(const std::any&)> ResolveFn;
		typedef std::function<void(const std::exception&)> RejectFn;
		typedef std::function<void()> FinallyFn;
		typedef std::function<void(ResolveFn, RejectFn)> TaskFn;
		struct TaskBaton
		{
			TaskFn fn;
			ResolveFn on_then;
			RejectFn on_catch;
			FinallyFn on_finally;
			bool has_result;
			std::any result;
			bool has_error;
			std::exception error;
		};

		Task(TaskFn fn) {
			m_fn = fn;
		}

		~Task() {

		}

		Task& Then(ResolveFn fn) {
			m_then = fn;
			return *this;
		}

		Task& Catch(RejectFn fn) {
			m_catch = fn;
			return *this;
		}

		Task& Finally(FinallyFn fn) {
			m_finally = fn;
			return *this;
		}

		void Run() {
			uv_work_t *m_work = new uv_work_t;
			memset(m_work, 0, sizeof(uv_work_t));
			TaskBaton *baton = new TaskBaton();
			baton->fn = m_fn;
			baton->on_then = m_then;
			baton->on_catch = m_catch;
			baton->on_finally = m_finally;
			baton->has_result = false;
			baton->has_error = false;

			m_work->data = baton;

			uv_queue_work(EventLoop::Get().Handle(), m_work, [](uv_work_t* req) {
				TaskBaton *baton = (TaskBaton*)req->data;

				baton->fn([baton](const std::any& result) {
					baton->has_error = false;
					baton->has_result = true;
					baton->result = result;
				}, [baton](const std::exception &err) {
					baton->has_error = true;
					baton->has_result = false;
					baton->error = err;
				});
			}, [](uv_work_t* req, int status) {
				TaskBaton *baton = (TaskBaton*)req->data;

				if (baton->has_error && baton->on_catch) {
					baton->on_catch(baton->error);
				}
				else if (baton->has_result && baton->on_then) {
					baton->on_then(baton->result);
				}

				if (baton->on_finally) {
					baton->on_finally();
				}

				delete baton;
				delete req;
			});
		}

	private:
		TaskFn m_fn;
		ResolveFn m_then;
		RejectFn m_catch;
		FinallyFn m_finally;
	};
}
