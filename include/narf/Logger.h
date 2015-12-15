/*----------------------------------------------------------------------------*/
/* Copyright (c) Creighton 2015. All Rights Reserved.                         */
/* Open Source Software - May be modified and shared but must                 */
/* be accompanied by the license file in the root source directory            */
/*----------------------------------------------------------------------------*/

#ifndef NARF_LOGGER_H_
#define NARF_LOGGER_H_

#include <map>
#include <vector>
#include <ostream>
#include <fstream>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>
#include <thread>
#include <algorithm>
#include <unordered_map>
#include <sys/stat.h>
#include "narf/format.h"
#include "narf/path.h"

#ifndef NARF_LOGGER_BASEPATH
#define NARF_LOGGER_BASEPATH ""
#endif

namespace narf {

class Logger {
	public:
		enum Level { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, DISABLED, INHERIT };
		static std::string levelNames[8];
		static std::map<std::string, Level> levelMap;

		class Entry {
			private:
				Level level;
				std::string text;
				std::chrono::steady_clock::duration time;
				Logger* baseLogger;
			public:
				Entry(Level level, std::string text, Logger* baseLogger, std::chrono::steady_clock::duration time);
				Entry(Level level, std::string text, Logger* baseLogger);
				Level getLevel();
				std::string getText();
				std::chrono::system_clock::time_point getTime();
				std::chrono::steady_clock::duration getRelTime() { return time; }
				Logger* getLogger();
				std::string getOutput(bool showTimestamp = true);
				friend std::ostream& operator<<(std::ostream& os, const Entry& entry);
		};

		class View {
			private:
				std::vector<int> entryIDs;
				std::vector<std::ostream*> outputs;
				std::map<std::string, Level> levels;
				Level defaultVerbosity;
				bool showingTimestamps;
				void rescan();
			public:
				View(Level verbosity = Level::WARN, bool showTimestamps = true) /*: defaultVerbosity(verbosity), showingTimestamps(showTimestamps)*/;
				~View();
				std::vector<Entry> getEntries(int start = 0, int count = -1);
				void dumpEntries(std::ostream* out, int start = 0, int count = -1);
				void dumpEntries(std::ostream& out, int start = 0, int count = -1);
				void dumpEntriesToAll(int start = 0, int count = -1);
				void notify(int newID, bool silent = false);
				void showTimestamps(bool newVal) { showingTimestamps = newVal; }
				void setVerbosity(std::string loggerName, Level newLevel);
				Level getVerbosity(std::string loggerName);
				Level getVerbosity(Logger* logger);
				void addOutputStream(std::ostream* out);
				void addOutputStream(std::ostream& out);
				void delOutputStream(std::ostream* out);
				void delOutputStream(std::ostream& out);
		};

		static std::vector<Entry> entries;
		static std::vector<View*> views;

	private:
		static std::unordered_map<std::string, Logger*> loggers;
		static std::map<std::string, Level> prefs;
		static std::chrono::steady_clock::time_point startTime;
		static Level defaultLevel;
		std::string name;
		Logger* parent;
		std::vector<std::ostream*> outputs;
		Level verbosity;
		Logger(std::string name, Logger* parent = nullptr) /*: name(name), parent(parent), verbosity(Level::INHERIT), timestamps(true)*/;
		std::mutex loggingMutex;

	public:
		static std::ostream* createFileLogger(std::string path);
		static Logger* getLogger(std::string name = "");
		static void reset();
		static void loadDefaultPrefs(const std::map<std::string, Level>& inPrefs);
		static void setDefaultLevel(Level level) { defaultLevel = level; }
		static Level getDefaultLevel() { return defaultLevel; }

		void log(Level level, std::string msg, Logger* baseLogger = nullptr);
		Logger* getSubLogger(std::string subName);
		std::string getName();
		Logger* getParent();
		Level getVerbosity();
		void setVerbosity(Level newLevel);
		void trace(std::string msg);
		void debug(std::string msg);
		void info(std::string msg);
		void warn(std::string msg);
		void error(std::string msg);
		void fatal(std::string msg);

};

std::ostream& operator<<(std::ostream& os, Logger::Entry& entry);
std::ostream& operator<<(std::ostream& os, Logger& logger);

}

#endif /* NARF_LOGGER_H_ */
