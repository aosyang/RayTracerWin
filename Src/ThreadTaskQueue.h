//=============================================================================
// ThreadTaskQueue.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include <queue>
#include <mutex>

template<typename T>
class ThreadTaskQueue
{
public:
	// Add a task to task queue
	void AddTask(T Task);

	// Get a task from task queue
	bool GetTask(T* OutTask);

private:
	std::queue<T> EnqueuedTasks;
	
	// Mutex for queue accessing
	std::mutex QueueMutex;
};

template<typename T>
void ThreadTaskQueue<T>::AddTask(T Task)
{
	std::lock_guard<std::mutex> Lock(QueueMutex);

	EnqueuedTasks.push(Task);
}

template<typename T>
bool ThreadTaskQueue<T>::GetTask(T* OutTask)
{
	std::lock_guard<std::mutex> Lock(QueueMutex);

	if (!EnqueuedTasks.empty() && OutTask)
	{
		*OutTask = EnqueuedTasks.front();
		EnqueuedTasks.pop();
		return true;
	}

	return false;
}
