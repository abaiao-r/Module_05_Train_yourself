#ifndef PARALLELFOR_HPP
#define PARALLELFOR_HPP

#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

/**
 * Runs fn(0) .. fn(count-1) on a pool of threads. Tasks must be independent.
 * The first exception thrown by a task stops the remaining ones and is
 * rethrown on the calling thread.
 */
template <typename Fn>
void parallelFor(int count, Fn fn)
{
	if (count <= 0)
		return;
	unsigned hw = std::thread::hardware_concurrency();
	int workers = std::min<int>(count, hw ? static_cast<int>(hw) : 1);
	if (workers <= 1)
	{
		for (int i = 0; i < count; ++i)
			fn(i);
		return;
	}

	std::atomic<int> next{0};
	std::atomic<bool> failed{false};
	std::exception_ptr firstError;
	std::mutex errMutex;

	auto work = [&]() {
		for (;;)
		{
			int i = next.fetch_add(1);
			if (i >= count || failed.load())
				return;
			try
			{
				fn(i);
			}
			catch (...)
			{
				std::lock_guard<std::mutex> lk(errMutex);
				if (!firstError)
					firstError = std::current_exception();
				failed.store(true);
				return;
			}
		}
	};

	std::vector<std::thread> pool;
	pool.reserve(static_cast<size_t>(workers - 1));
	for (int t = 1; t < workers; ++t)
		pool.emplace_back(work);
	work();
	for (auto &th : pool)
		th.join();
	if (firstError)
		std::rethrow_exception(firstError);
}

#endif
