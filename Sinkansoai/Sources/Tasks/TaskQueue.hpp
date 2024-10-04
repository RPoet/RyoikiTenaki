#pragma once
#include "TaskQueue.h"

template< class TCommandType > 
int32 TTaskQueue< TCommandType >::Main()
{
	RRenderCommandList GlobalCommandList{};

	while (TaskQueue.size() > 0)
	{
		auto ExecutionCommandName = CommandNames.front();

		auto Task = TaskQueue.front();
		TaskQueue.pop();

		Task(GlobalCommandList);

		CommandNames.pop();
	}

	return 0;
}

template< class TCommandType >
void TTaskQueue< TCommandType >::AddRenderCommand(String&& CommandName, TCommandType&& CommandType)
{
	TaskQueue.emplace(std::forward( CommandType ));
	CommandNames.emplace(std::move( CommandName ));
}
