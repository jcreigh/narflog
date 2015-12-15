/*----------------------------------------------------------------------------*/
/* Copyright (c) Creighton 2015. All Rights Reserved.                         */
/* Open Source Software - May be modified and shared but must                 */
/* be accompanied by the license file in the root source directory            */
/*----------------------------------------------------------------------------*/

#include "narf/Logger.h"

namespace narf {

Logger::Entry::Entry(Level level, std::string text, narf::Logger* baseLogger, std::chrono::steady_clock::duration time)
: level(level), text(text), time(time), baseLogger(baseLogger) {}

Logger::Entry::Entry(Level level, std::string text, narf::Logger* baseLogger)
	: level(level), text(text), baseLogger(baseLogger) {
	time = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::steady_clock::now() - startTime);
}

Logger::Level Logger::Entry::getLevel() {
	return level;
}

std::string Logger::Entry::getText() {
	return text;
}

std::chrono::system_clock::time_point Logger::Entry::getTime() {
	return std::chrono::system_clock::now() - (std::chrono::steady_clock::now() - startTime) + time;
}

narf::Logger* Logger::Entry::getLogger() {
	return baseLogger;
}

std::string Logger::Entry::getOutput(bool showTimestamp /* = true */) {
	std::string outMsg = "[" + levelNames[level] + "] " + ((baseLogger && baseLogger->getName().size()) ? ("[" + baseLogger->getName() + "] ") : "") + text + "\n";
	if (showTimestamp) {
		outMsg = narf::util::format("[%.3f] ", (double)(std::chrono::duration_cast<std::chrono::milliseconds>(getTime().time_since_epoch()).count()) / 1000) + outMsg;
	}
	return outMsg;
}

std::ostream& operator<<(std::ostream& os, Logger::Entry& entry) {
	os << entry.getOutput();
	return os;
}

}
