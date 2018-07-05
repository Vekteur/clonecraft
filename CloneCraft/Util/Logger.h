#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#undef ERROR
enum class Level {
	DEBUG, TRACE, ERROR, WARNING, INFO, OFF
};

class Logger
{
public:
	Logger(Level standardOutputLevel = Level::DEBUG, Level fileOutputLevel = Level::OFF);

	void setOutputFile(std::string file);
	std::string getOutputFile();

	void setStandardOutputLevel(Level level);
	void setFileOutputLevel(Level level);
	Level standardOutputLevel();
	Level fileOutputLevel();
	bool isStandardOutputLevelActive(Level level);
	bool isFileOutputLevelActive(Level level);

	Logger& operator()(Level level);

	template <typename T>
	friend Logger& operator<<(Logger& logger, const T& message);

	using coutType = std::basic_ostream<char, std::char_traits<char> >;
	using standardEndLine = coutType&(*)(coutType&);
	Logger& operator<<(standardEndLine manip);

private:
#ifdef _DEBUG
	static const bool debugMode = true;
#else
	static const bool debugMode = false;
#endif
	const std::string defaultOutputFile = "log";

	Level activeLevel = Level::INFO;
	Level minStandardOutputLevel;
	Level minFileOutputLevel;
	std::ofstream fstream;
	std::string outputFile = defaultOutputFile;
	std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();

	std::string timeFromStart();
};

template<typename T>
Logger& operator<<(Logger& logger, const T& message) {
	if (logger.isStandardOutputLevelActive(logger.activeLevel)) {
		std::cout << message;
	}
	if (logger.isFileOutputLevelActive(logger.activeLevel)) {
		logger.fstream << message;
	}
	return logger;
}

extern Logger LOG;