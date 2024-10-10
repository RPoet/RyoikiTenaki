#pragma once
#include "TaskQueue.h"
#include "../RenderBackend/RenderBackendCommon.h"

template< class TCommandType > 
int32 TTaskQueue< TCommandType >::Main()
{
	while (TaskQueue.size() > 0)
	{
		if (!GBackend)
		{
			break;
		}

		//cout << CommandNames.front() << endl;
		TaskQueue.front()(*GBackend->GetMainCommandList());
		//cout << CommandNames.front() << endl;

		TaskQueue.pop();
		CommandNames.pop();
	}

	return 0;
}

template< class TCommandType >
void TTaskQueue< TCommandType >::AddRenderCommand(String&& CommandName, TCommandType&& CommandType)
{
	TaskQueue.emplace(CommandType);
	CommandNames.emplace(std::move( CommandName ));
}
