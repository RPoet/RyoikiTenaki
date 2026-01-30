#pragma once
#include "../Singleton.h"
#include "TaskQueue.h"
#include "../RenderBackend/RenderCommandList.h"

class MTaskSystem : public Singleton<MTaskSystem>
{
private:

	using RenderCommand = std::function<void(RGraphicsCommandList&)>;
	TTaskQueue<RenderCommand> RenderCommandQueue;

	//using TaskCommand = void(*)(void);
	//TTaskQueue<TaskCommand> TaskCommandQueue;
public:

	MTaskSystem() = default;

	void LaunchTasks();

	void AddRenderCommand(String&& CommandName, RenderCommand&& CommandType)
	{
		RenderCommandQueue.AddRenderCommand(std::move(CommandName), std::move(CommandType));
	}
};
