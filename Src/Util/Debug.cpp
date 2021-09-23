#include "Debug.h"

bool debug::no_recent_success(int interval_ms) {
	static StopWatch<std::chrono::milliseconds, std::chrono::steady_clock> stop_watch;
	if (stop_watch.elapsed() < interval_ms)
		return false;
	stop_watch.restart();
	return true;
}
