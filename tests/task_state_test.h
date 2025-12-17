#pragma once

#include "common/eqemu_logsys.h"
#include "common/shared_tasks.h"
#include "common/tasks.h"
#include "cppunit/cpptest.h"

class TaskStateTest: public Test::Suite
{
public:
	TaskStateTest()
	{
		TEST_ADD(TaskStateTest::TestSequenceMode);
		TEST_ADD(TaskStateTest::TestSteps);
		TEST_ADD(TaskStateTest::TestStepGaps);
		TEST_ADD(TaskStateTest::TestUnorderedSteps);
		TEST_ADD(TaskStateTest::TestOptionalSteps);
		TEST_ADD(TaskStateTest::TestOptionalLastSteps);
		TEST_ADD(TaskStateTest::TestOptionalSequence);
		TEST_ADD(TaskStateTest::TestWorldTemplateSupport);
		TEST_ADD(TaskStateTest::TestReqActivityID);
		TEST_ADD(TaskStateTest::TestReqActivityIDOverrideStep);
		TEST_ADD(TaskStateTest::TestReqActivityIDSteps);
		TEST_ADD(TaskStateTest::TestReqActivityIDUnorderedSteps);
		TEST_ADD(TaskStateTest::TestReqActivityIDMixSteps);
		TEST_ADD(TaskStateTest::TestReqActivityIDSequenceMode);
		TEST_ADD(TaskStateTest::TestReqActivityIDOptional);
		TEST_ADD(TaskStateTest::TestReqActivityIDOptionalLastSteps);
	}

private:
	void TestSequenceMode();
	void TestSteps();
	void TestStepGaps();
	void TestUnorderedSteps();
	void TestOptionalSteps();
	void TestOptionalLastSteps();
	void TestOptionalSequence();
	void TestWorldTemplateSupport();
	void TestReqActivityID();
	void TestReqActivityIDOverrideStep();
	void TestReqActivityIDSteps();
	void TestReqActivityIDUnorderedSteps();
	void TestReqActivityIDMixSteps();
	void TestReqActivityIDSequenceMode();
	void TestReqActivityIDOptional();
	void TestReqActivityIDOptionalLastSteps();

	TaskInformation GetMockZoneData(int count)
	{
		TaskInformation task;
		task.activity_count = count;
		for (int i = 0; i < task.activity_count; ++i)
		{
			task.activity_information[i].req_activity_id = -1;
			task.activity_information[i].step = 0;
			task.activity_information[i].optional = false;
		}
		return task;
	}

	ClientTaskInformation GetMockZoneState(int count)
	{
		ClientTaskInformation state;
		for (int i = 0; i < count; ++i)
		{
			state.activity[i].activity_id = i;
			state.activity[i].activity_state = ActivityState::ActivityHidden;
		}
		return state;
	}

	std::vector<TaskActivitiesRepository::TaskActivities> GetMockWorldData(int count)
	{
		std::vector<TaskActivitiesRepository::TaskActivities> data;
		data.resize(count);
		for (int i = 0; i < count; ++i)
		{
			data[i].activityid = i;
			data[i].req_activity_id = -1;
			data[i].step = 0;
			data[i].optional = false;
		}
		return data;
	}

	std::vector<SharedTaskActivityStateEntry> GetMockWorldState(int count)
	{
		std::vector<SharedTaskActivityStateEntry> states;
		states.resize(count);
		for (int i = 0; i < count; ++i)
		{
			states[i].activity_id = i;
			states[i].req_activity_id = -1;
			states[i].step = 0;
			states[i].optional = false;
			states[i].activity_state = ActivityState::ActivityHidden;
		}
		return states;
	}
};

void TaskStateTest::TestSequenceMode()
{
	int activity_count = 3;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | hidden (active)
		// 1 | 0 | hidden
		// 2 | 0 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 0 | hidden (active)
		// 2 | 0 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id |step | state
		// 0 | 0 | completed
		// 1 | 0 | completed
		// 2 | 0 | hidden (active)

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 0 | completed
		// 2 | 0 | completed

		// task completed, none should be active
		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityHidden;
		data.activity_information[2].optional = true;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 0 | completed
		// 2 | 0 | hidden | optional (active)

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}
}

void TaskStateTest::TestSteps()
{
	int activity_count = 4;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[1].step = 1;
	data.activity_information[2].step = 1;
	data.activity_information[3].step = 2;

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | hidden (active)
		// 1 | 1 | hidden
		// 2 | 1 | hidden
		// 3 | 2 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 1 | hidden (active)
		// 2 | 1 | hidden (active)
		// 3 | 2 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 1 | completed
		// 2 | 1 | hidden (active)
		// 3 | 2 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 1 | completed
		// 2 | 1 | completed
		// 3 | 2 | hidden (active)

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 1 | completed
		// 2 | 1 | completed
		// 3 | 2 | completed

		// should be complete with none active
		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityHidden;
		data.activity_information[3].optional = true;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// activity_id | step | state
		// 0 | 0 | completed
		// 1 | 1 | completed
		// 2 | 1 | completed
		// 3 | 2 | hidden | optional (active)

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}
}

void TaskStateTest::TestStepGaps()
{
	int activity_count = 5;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 1;
	data.activity_information[1].step = 5;
	data.activity_information[2].step = 5;
	data.activity_information[3].step = 100;
	data.activity_information[4].step = 100;

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 0 should be active starting at step 1
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// indexes 1 and 2 should be active at step 5
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// only index 2 should be active with step 5 since index 1 is completed
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// indexes 3 and 4 should be active for step 100
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;
		state.activity[4].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}

void TaskStateTest::TestUnorderedSteps()
{
	int activity_count = 5;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 100;
	data.activity_information[1].step = 100;
	data.activity_information[2].step = 3;
	data.activity_information[3].step = 20;
	data.activity_information[4].step = 1;

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 4 should be active as the lowest step
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[4].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 2 should be the next lowest step (3) after step 1 completed
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[4].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 3 should be active as step 20
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[4].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// indexes 0 and 1 should both be active as step 100
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}
}

void TaskStateTest::TestOptionalSteps()
{
	int activity_count = 4;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 0;
	data[1].step = 1;
	data[1].optional = true;
	data[2].step = 2;
	data[2].optional = true;
	data[3].step = 2;

	{
		auto res = Tasks::GetActiveElements(data, state, activity_count);

		// activity_id | step | state
		// 0 | 0 | hidden (active)
		// 1 | 1 | hidden | optional
		// 2 | 2 | hidden | optional
		// 3 | 2 | hidden

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 1 | hidden | optional (active)
		// 2 | 2 | hidden | optional (active)
		// 3 | 2 | hidden (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		// steps that only contain optionals should not need to be completed to open next step
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 3);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 1 | complete | optional
		// 2 | 2 | hidden | optional  (active)
		// 3 | 2 | hidden (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;
		state[3].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 1 | complete | optional
		// 2 | 2 | hidden | optional  (active)
		// 3 | 2 | complete

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;
		state[2].activity_state = ActivityState::ActivityCompleted;
		state[3].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}

void TaskStateTest::TestOptionalLastSteps()
{
	int activity_count = 3;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 0;
	data[1].optional = true;
	data[1].step = 1;
	data[2].optional = true;
	data[2].step = 2;

	{
		// activity_id | step | state
		// 0 | 0 | hidden (active)
		// 1 | 1 | hidden | optional
		// 2 | 2 | hidden | optional

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 1 | hidden | optional (active)
		// 2 | 2 | hidden | optional (active)

		// step with only an optional should not prevent next step being active
		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 1 | complete | optional
		// 2 | 2 | hidden | optional (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}
}

void TaskStateTest::TestOptionalSequence()
{
	int activity_count = 3;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 0;
	data[1].step = 0;
	data[1].optional = true;
	data[2].step = 0;

	{
		// activity_id | step | state
		// 0 | 0 | hidden (active)
		// 1 | 0 | hidden | optional
		// 2 | 0 | hidden

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 0 | hidden | optional (active)
		// 2 | 0 | hidden (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 0 | complete
		// 2 | 0 | hidden (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityHidden;
		state[2].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | 0 | complete
		// 1 | 0 | hidden | optional (active)
		// 2 | 0 | complete

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;
		state[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}

void TaskStateTest::TestWorldTemplateSupport()
{
	int activity_count = 3;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 1;
	data[1].step = 5;
	data[2].step = 10;
	state[0].step = 1;
	state[1].step = 5;
	state[2].step = 10;
	state[0].activity_state = ActivityState::ActivityCompleted;

	{
		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		// using the state struct as both data and state source
		auto res = Tasks::GetActiveElements(state, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}
}

void TaskStateTest::TestReqActivityID()
{
	int activity_count = 5;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[1].req_activity_id = 0;
	data.activity_information[2].req_activity_id = 0;
	data.activity_information[3].req_activity_id = 1;
	data.activity_information[4].req_activity_id = 2;

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;
		state.activity[4].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}

void TaskStateTest::TestReqActivityIDOverrideStep()
{
	int activity_count = 4;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 0;
	data.activity_information[1].step = 1;
	data.activity_information[2].req_activity_id = 1;
	data.activity_information[2].step = 1;
	data.activity_information[3].req_activity_id = 1;
	data.activity_information[3].step = 1;

	state.activity[0].activity_state = ActivityState::ActivityCompleted;

	auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

	// indexes 2 and 3 should require index 1 to be completed instead of activating with step 1
	TEST_ASSERT(res.is_task_complete == false);
	TEST_ASSERT(res.active.size() == 1);
	TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
}

void TaskStateTest::TestReqActivityIDSteps()
{
	int activity_count = 5;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 0;
	data.activity_information[1].step = 1;
	data.activity_information[2].req_activity_id = 0;
	data.activity_information[2].step = 2;
	data.activity_information[3].req_activity_id = 0;
	data.activity_information[3].step = 2;
	data.activity_information[4].step = 3;

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 1 should become active as next step, indexes 2 and 3 because of reqs
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 3);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | complete
		// 2 |  0 | 2 | hidden (active)
		// 3 |  0 | 2 | hidden (active)
		// 4 | -1 | 3 | hidden

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 4 (step 3) should not become active until step 2 is completed
		// even though index 2 and 3 are active because of reqs
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// should still be on step 2
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;
		state.activity[4].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}


void TaskStateTest::TestReqActivityIDUnorderedSteps()
{
	int activity_count = 5;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 0;
	data.activity_information[1].step = 1;
	data.activity_information[2].req_activity_id = 0;
	data.activity_information[2].step = 0;
	data.activity_information[3].req_activity_id = 0;
	data.activity_information[3].step = 0;
	data.activity_information[4].step = 3;

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | hidden
		// 2 |  0 | 0 | hidden (active)
		// 3 |  0 | 0 | hidden (active)
		// 4 | -1 | 3 | hidden

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | hidden (active)
		// 2 |  0 | 0 | complete
		// 3 |  0 | 0 | complete
		// 4 | -1 | 3 | hidden

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | complete
		// 2 |  0 | 0 | complete
		// 3 |  0 | 0 | complete
		// 4 | -1 | 3 | hidden (active)

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// should still be on step 3
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}
}

void TaskStateTest::TestReqActivityIDMixSteps()
{
	int activity_count = 6;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[0].step = 0;
	data.activity_information[1].step = 1;
	data.activity_information[2].step = 2;
	data.activity_information[3].req_activity_id = 0;
	data.activity_information[3].step = 2;
	data.activity_information[4].req_activity_id = 0;
	data.activity_information[4].step = 2;
	data.activity_information[5].step = 3;

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | complete
		// 2 | -1 | 2 | hidden (active)
		// 3 |  0 | 2 | hidden (active)
		// 4 |  0 | 2 | hidden (active)
		// 5 | -1 | 3 | hidden

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 3);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | complete
		// 2 | -1 | 2 | complete
		// 3 |  0 | 2 | hidden (active)
		// 4 |  0 | 2 | hidden (active)
		// 5 | -1 | 3 | hidden

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 5 (step 3) should not be active yet after completing only index with non-req id
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 2);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 3) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;
		state.activity[2].activity_state = ActivityState::ActivityCompleted;
		state.activity[3].activity_state = ActivityState::ActivityCompleted;
		state.activity[4].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 | -1 | 1 | complete
		// 2 | -1 | 2 | complete
		// 3 |  0 | 2 | complete
		// 4 |  0 | 2 | complete
		// 5 | -1 | 3 | hidden (active)

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 5) != res.active.end());
	}
}

void TaskStateTest::TestReqActivityIDSequenceMode()
{
	int activity_count = 6;
	TaskInformation data = GetMockZoneData(activity_count);
	ClientTaskInformation state = GetMockZoneState(activity_count);
	data.activity_information[4].req_activity_id = 1;
	data.activity_information[5].req_activity_id = 0;

	{
		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state.activity[0].activity_state = ActivityState::ActivityCompleted;
		state.activity[1].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data.activity_information, state.activity, activity_count);

		// index 2 because of sequence mode, index 4 and 5 due to req indexes being complete
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 3);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 4) != res.active.end());
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 5) != res.active.end());
	}
}

void TaskStateTest::TestReqActivityIDOptional()
{
	int activity_count = 3;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 0;
	data[1].req_activity_id = 0;
	data[1].step = 1;
	data[1].optional = true;
	data[2].req_activity_id = 1;
	data[2].step = 2;

	{
		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | hidden (active)
		// 1 |  0 | 1 | hidden | optional
		// 2 |  1 | 2 | hidden

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 |  0 | 1 | hidden | optional (active)
		// 2 |  1 | 2 | hidden

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		// index 1 is effectively non-optional since non-optional index 2 requires it
		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 |  0 | 1 | complete | optional
		// 2 |  1 | 2 | hidden (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;
		state[2].activity_state = ActivityState::ActivityCompleted;

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.empty());
	}
}

void TaskStateTest::TestReqActivityIDOptionalLastSteps()
{
	int activity_count = 3;
	auto data = GetMockWorldData(activity_count);
	auto state = GetMockWorldState(activity_count);
	data[0].step = 0;
	data[1].req_activity_id = 0;
	data[1].step = 1;
	data[1].optional = true;
	data[2].req_activity_id = 1;
	data[2].step = 2;
	data[2].optional = true;

	{
		// activity_id | req_activity_id | step | state
		// 0 | -1 | 0 | hidden (active)
		// 1 |  0 | 1 | hidden | optional
		// 2 |  1 | 2 | hidden | optional

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == false);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 0) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 |  0 | 1 | hidden | optional (active)
		// 2 |  1 | 2 | hidden | optional

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 1) != res.active.end());
	}

	{
		state[0].activity_state = ActivityState::ActivityCompleted;
		state[1].activity_state = ActivityState::ActivityCompleted;

		// activity_id | step | state
		// 0 | -1 | 0 | complete
		// 1 |  0 | 1 | complete | optional
		// 2 |  1 | 2 | hidden | optional (active)

		auto res = Tasks::GetActiveElements(data, state, activity_count);

		TEST_ASSERT(res.is_task_complete == true);
		TEST_ASSERT(res.active.size() == 1);
		TEST_ASSERT(std::find(res.active.begin(), res.active.end(), 2) != res.active.end());
	}
}
