#pragma once
#include "../Definitions.h"
#include "../RenderCommandList.h"

template<class TCommandType>
class TTaskQueue
{
private:
	queue<TCommandType> TaskQueue;
	queue<String> CommandNames;

public:
	TTaskQueue() = default;
	int32 Main();


	void AddRenderCommand(String&& CommandName, TCommandType&& CommandType);
};

#include "TaskQueue.hpp"
