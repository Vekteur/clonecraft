#include "Logger.h"

template<typename E>
constexpr auto toInt(E e) -> typename std::underlying_type<E>::type {
	return static_cast<typename std::underlying_type<E>::type>(e);
}

Logger::Logger(Level standardOutputLevel, Level fileOutputLevel) {
	setStandardOutputLevel(standardOutputLevel);
	setFileOutputLevel(fileOutputLevel);
}

void Logger::setOutputFile(std::string file) {
	outputFile = file;
	fstream = std::ofstream{ file };
}

std::string Logger::getOutputFile() {
	return outputFile;
}

void Logger::setStandardOutputLevel(Level level) {
	if (!debugMode && toInt(level) <= toInt(Level::DEBUG)) {
		minStandardOutputLevel = static_cast<Level>(toInt(Level::DEBUG) + 1);
	} else {
		minStandardOutputLevel = level;
	}
}

void Logger::setFileOutputLevel(Level level) {
	if (!debugMode && toInt(level) <= toInt(Level::DEBUG)) {
		minFileOutputLevel = static_cast<Level>(toInt(Level::DEBUG) + 1);
	} else {
		minFileOutputLevel = level;
	}
}

Level Logger::standardOutputLevel() {
	return minStandardOutputLevel;
}

Level Logger::fileOutputLevel() {
	return minFileOutputLevel;
}

bool Logger::isStandardOutputLevelActive(Level level) {
	return toInt(minStandardOutputLevel) <= toInt(level);
}

bool Logger::isFileOutputLevelActive(Level level) {
	return toInt(minFileOutputLevel) <= toInt(level);
}

std::string Logger::timeFromStart() {
	auto time = std::chrono::system_clock::now() - startTime;
	auto sTime = std::chrono::duration_cast<std::chrono::seconds>(time);
	auto microTime = std::chrono::duration_cast<std::chrono::microseconds>(time - sTime);
	std::ostringstream oss;
	oss << sTime.count() << ',' << microTime.count();
	return oss.str();
}

Logger& Logger::operator()(Level level) {
	activeLevel = level;
	(*this) << timeFromStart() << ' ';
	return *this;
}

Logger & Logger::operator<<(standardEndLine manip) {
	if (isStandardOutputLevelActive(activeLevel)) {
		manip(std::cout);
	}
	if (isFileOutputLevelActive(activeLevel)) {
		manip(fstream);
	}
	return *this;
}

Logger LOG;