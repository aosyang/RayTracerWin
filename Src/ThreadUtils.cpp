//=============================================================================
// ThreadUtils.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "ThreadUtils.h"

#include <assert.h>

ScopeAutoJoinedThreads::~ScopeAutoJoinedThreads()
{
	for (auto& Thread : Threads)
	{
		// Thread must not be detached
		assert(Thread.joinable());
		Thread.join();
	}
}

void ScopeAutoJoinedThreads::AddThread(std::thread& Thread)
{
	Threads.push_back(std::move(Thread));
}
