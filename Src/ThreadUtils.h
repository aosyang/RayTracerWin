//=============================================================================
// ThreadUtils.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include <vector>
#include <thread>

// Collect a list of threads and join them when instance leaves the scope
class ScopeAutoJoinedThreads
{
public:
	~ScopeAutoJoinedThreads();

	// Add a thread to auto joined thread list
	void AddThread(std::thread& Thread);

private:
	// All threads to be joined
	std::vector<std::thread> Threads;
};
