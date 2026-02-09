#pragma once
#include "TaskQueue.h"
#include "../RenderBackend/RenderBackendCommon.h"

template< class TCommandType > 
int32 TTaskQueue< TCommandType >::Main()
{
	if (TaskQueue.size() == 0 || !GBackend)
	{
		return 0;
	}

	GBackend->RenderBegin();

	while (TaskQueue.size() > 0)
	{
		TaskQueue.front()(*GBackend->GetMainGraphicsCommandList());

		TaskQueue.pop();
		CommandNames.pop();
	}

	GBackend->RenderFinish();

	return 0;
}

template< class TCommandType >
void TTaskQueue< TCommandType >::AddRenderCommand(String&& CommandName, TCommandType&& CommandType)
{
	TaskQueue.emplace(CommandType);
	CommandNames.emplace(std::move(CommandName));
}

