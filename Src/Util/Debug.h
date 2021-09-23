#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <numeric>
#include <chrono>
#include <functional>

namespace debug {
// Mean and standard error of the mean
template<typename T>
std::tuple<T, T> mean_and_its_error(const std::vector<T>& v) {
	T mean = std::accumulate(v.begin(), v.end(), 0) / v.size();
	T var = 0;
	int n = v.size();
	if (n > 1) {
		for (T x : v) {
			T diff = mean - x;
			var += diff * diff;
		}
		var /= n - 1;
	}
	T error = sqrt(var / n); // Standard error
	return { mean, error };
}

template<typename TimeT = std::chrono::milliseconds>
typename std::tuple<typename TimeT::rep, typename TimeT::rep>
measure_time(std::function<void(void)> func, int repeat = 1) {

	std::vector<typename TimeT::rep> times;
	for (int i = 0; i < repeat; ++i) {
		auto start = std::chrono::steady_clock::now();
		func();
		auto elapsed = std::chrono::steady_clock::now() - start;
		times.push_back(std::chrono::duration_cast<TimeT>(elapsed).count());
	}
	return mean_and_its_error(times);
}

template<typename TimeT>
std::string time_unit_str() {
	std::string time_unit = "?";
	if (std::is_same<TimeT, std::chrono::nanoseconds>::value)
		time_unit = "ns";
	else if (std::is_same<TimeT, std::chrono::microseconds>::value)
		time_unit = "us";
	else if (std::is_same<TimeT, std::chrono::milliseconds>::value)
		time_unit = "ms";
	else if (std::is_same<TimeT, std::chrono::seconds>::value)
		time_unit = "s";
	return time_unit;
}

template<typename TimeT = std::chrono::milliseconds>
struct FunctionTimer {
	int m_repeat;
	int m_mean, m_error;

	FunctionTimer(std::function<void(void)> func, int repeat = 1) : m_repeat{ repeat } {
		std::tie(m_mean, m_error) = measure_time<TimeT>(func, repeat);
	}
};

template<typename TimeT = std::chrono::milliseconds, typename ClockT = std::chrono::steady_clock>
struct StopWatch {
	std::chrono::time_point<ClockT> start = ClockT::now();

	void restart() {
		start = ClockT::now();
	}

	typename TimeT::rep elapsed() const {
		return std::chrono::duration_cast<TimeT>(ClockT::now() - start).count();
	}
};

template<typename TimeT>
std::ostream& operator<<(std::ostream& out, const FunctionTimer<TimeT>& timer) {
	out << timer.m_mean;
	if (timer.m_repeat != 1) {
		out << " +- " << timer.m_error;
	}
	out << " " << time_unit_str<TimeT>();
	return out;
}

template<typename TimeT, typename ClockT>
std::ostream& operator<<(std::ostream& out, const StopWatch<TimeT, ClockT>& stop_watch) {
	out << stop_watch.elapsed() << " " << time_unit_str<TimeT>();
	return out;
}

bool no_recent_success(int interval_ms = 1000);
}