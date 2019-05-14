//=============================================================================
// ThreadUtils.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "ThreadUtils.h"

#include <assert.h>
#include "MathHelper.h"

namespace ThreadUtils
{
	int DetectWorkerThreadsNum()
	{
		int ThreadConcurrency = (int)std::thread::hardware_concurrency();
		return Math::Max(ThreadConcurrency, 1);
	}
}

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
