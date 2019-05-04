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
	ThreadTaskQueue()
		: bQuit(false)
	{
	}

	// Add a task to task queue
	void PushTask(T Task);

	// Get a task from task queue
	bool PopTask(T* OutTask);

	int GetNumTasks() const
	{
		return (int)EnqueuedTasks.size();
	}

	std::mutex& GetMutex()
	{
		return QueueMutex;
	}

	std::condition_variable& GetWorkerThreadCondition()
	{
		return WorkerThreadCondition;
	}

	// Notify the task queue about one task finished
	void NotifySingleTaskDone()
	{
		QueueCondition.notify_one();
	}

	void NotifyQuit()
	{
		bQuit = true;
		QueueCondition.notify_all();
		WorkerThreadCondition.notify_all();
	}

	void WaitForAllTasksDone()
	{
		std::unique_lock<std::mutex> ThreadLock(QueueMutex);

		// Wait until task queue is empty
		QueueCondition.wait(ThreadLock, [this] {
			return EnqueuedTasks.size() == 0 || bQuit;
		});
	}

private:
	std::queue<T> EnqueuedTasks;
	
	// Mutex for queue accessing
	std::mutex QueueMutex;
	std::condition_variable QueueCondition;
	std::condition_variable WorkerThreadCondition;

	bool bQuit : 1;
};

template<typename T>
void ThreadTaskQueue<T>::PushTask(T Task)
{
	{
		std::lock_guard<std::mutex> Lock(QueueMutex);
		EnqueuedTasks.push(Task);
	}

	WorkerThreadCondition.notify_one();
}

template<typename T>
bool ThreadTaskQueue<T>::PopTask(T* OutTask)
{
	if (!EnqueuedTasks.empty() && OutTask)
	{
		*OutTask = EnqueuedTasks.front();
		EnqueuedTasks.pop();
		return true;
	}

	return false;
}
