/*----------------------------------------------------------------------------*/
/* Copyright (c) Creighton 2015. All Rights Reserved.                         */
/* Open Source Software - May be modified and shared but must                 */
/* be accompanied by the license file in the root source directory            */
/*----------------------------------------------------------------------------*/

#include <iostream>
#include "narf/Logger.h"

namespace narf {

std::unordered_map<std::string, Logger*> Logger::loggers;
std::chrono::steady_clock::time_point Logger::startTime = std::chrono::steady_clock::now();
std::vector<Logger::Entry> Logger::entries;
std::vector<Logger::View*> Logger::views;
std::map<std::string, Logger::Level> Logger::prefs;
Logger::Level Logger::defaultLevel = Logger::Level::INHERIT;
narf::Signal<void (int)> Logger::globalSignal;

std::string rotateFile(std::string path, int n = 0) {
	std::string newPath = path + (n > 0 ? narf::util::format(".%d", n) : "");
	if (narf::util::fileExists(newPath)) {
		narf::util::rename(path, rotateFile(path, n + 1));
	}
	return newPath;
}

std::ostream* Logger::createFileLogger(std::string path) {
	std::string parentPath = narf::util::baseName(path);
	if (parentPath != "" && !narf::util::dirExists(parentPath)) {
		if (!narf::util::createDirs(parentPath)) {
			// Failed to create the log directory, so just return a stringstream to eat everything.
			// There's probably something better which will actually eat it
			// Maybe output something to stderr?
			return new std::stringstream();
		}
	}
	return new std::ofstream(rotateFile(path));
}

Logger* Logger::getLogger(std::string name /*= ""*/) {
	if (loggers.count(name) == 0) {
		auto p = name.rfind(".");
		std::string parent = (p != std::string::npos) ? name.substr(0, p) : "";
		Logger* parentLog = (name != "") ? getLogger(parent) : nullptr;
		loggers[name] = new Logger(name, parentLog);
	}
	return loggers[name];
}

void Logger::reset() {
	entries.clear();
}

void Logger::loadDefaultPrefs(const std::map<std::string, Level>& inPrefs) {
	for (auto& pair : inPrefs) {
		prefs[pair.first] = pair.second;
	}
}

Logger::Logger(std::string name, Logger* parent /*= nullptr*/) : name(name), parent(parent), verbosity(Logger::Level::INHERIT) {
	auto pref = prefs.find(name);
	if (pref != prefs.end()) {
		verbosity = pref->second;
		//printf("Initializing logger \"%s\" at %s (from pref)\n", name.c_str(), levelNames[verbosity].c_str());
		return;
	} else {
		verbosity = defaultLevel;
	}
	//printf("Initializing logger \"%s\" at %s\n", name.c_str(), levelNames[verbosity].c_str());
}

void Logger::log(Logger::Level level, std::string msg, Logger* baseLogger /*= nullptr*/) {
	loggingMutex.lock();

	if (baseLogger == nullptr) {
		baseLogger = this;
	}

	auto verbosity_ = getVerbosity();

	if (level < verbosity_ || verbosity_ == Level::DISABLED) {
		loggingMutex.unlock();
		return;
	}

	auto entry = Entry(level, msg, baseLogger);
	entries.push_back(entry);
	int newestID = (int)entries.size() - 1;
	loggingMutex.unlock();

	signal.emit(newestID);
	globalSignal.emit(newestID);

	for (auto& view : views) {
		view->notify(newestID);
	}

}

Logger* Logger::getSubLogger(std::string subName) {
	return getLogger((name.size() > 0 ? (name + ".") : "") + subName);
}

std::string Logger::getName() {
	return name;
}

Logger* Logger::getParent() {
	return parent;
}

Logger::Level Logger::getVerbosity() {
	if (verbosity == Logger::Level::INHERIT) {
		if (parent != nullptr) {
			return parent->getVerbosity();
		} else { // We're the root logger, if we don't have anything set, default to TRACE
			return Level::TRACE;
		}
	}
	return verbosity;
}

void Logger::setVerbosity(Logger::Level newLevel) {
	verbosity = newLevel;
}

void Logger::trace(std::string msg) {
	log(Level::TRACE, msg);
}

void Logger::debug(std::string msg) {
	log(Level::DEBUG, msg);
}

void Logger::info(std::string msg) {
	log(Level::INFO, msg);
}

void Logger::warn(std::string msg) {
	log(Level::WARN, msg);
}

void Logger::error(std::string msg) {
	log(Level::ERROR, msg);
}

void Logger::fatal(std::string msg) {
	log(Level::FATAL, msg);
}

}
