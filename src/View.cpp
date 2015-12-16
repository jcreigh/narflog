/*----------------------------------------------------------------------------*/
/* Copyright (c) Creighton 2015. All Rights Reserved.                         */
/* Open Source Software - May be modified and shared but must                 */
/* be accompanied by the license file in the root source directory            */
/*----------------------------------------------------------------------------*/

#include "narf/Logger.h"

namespace narf {

Logger::View::View(Logger::Level verbosity /* = Logger::Level::WARN */, bool showTimestamps /* = true */) : defaultVerbosity(verbosity), showingTimestamps(showTimestamps){
	rescan();
	slotID = Logger::globalSignal += [this](int newID) { this->notify(newID); };
	//Logger::views.push_back(this);
}

Logger::View::~View() {
	Logger::globalSignal -= slotID;
	//Logger::views.erase(std::remove(Logger::views.begin(), Logger::views.end(), this), Logger::views.end());
}

void Logger::View::dumpEntries(std::ostream* out, int start /* = 0 */, int count /* = -1 */) {
	auto entries = getEntries(start, count);
	for (auto& entry : entries) {
		(*out) << entry.getOutput(showingTimestamps);
	}
}

void Logger::View::dumpEntries(std::ostream& out, int start /* = 0 */, int count /* = -1 */) {
	dumpEntries(&out, start, count);
}

void Logger::View::dumpEntriesToAll(int start /* = 0 */, int count /* = -1 */) {
	for (auto& out : outputs) {
		dumpEntries(out, start, count);
	}
}

std::vector<Logger::Entry> Logger::View::getEntries(int start /* = 0 */, int count /* = -1 */) {
	if (count < 0) { // If count < 0, then we want to show the entries up to the end
		count = (int)entryIDs.size();
	}
	if (start < 0) { // If start < 0, we want to start counting back from the most recent entry
		start = (int)entryIDs.size() - start;
	}

	std::vector<Logger::Entry> out;
	for (int i = 0; ((start + i) < (int)entryIDs.size()) && (i < count); i++) {
		out.push_back(Logger::entries[entryIDs[start + i]]);
	}
	return out;
}

void Logger::View::notify(int newID, bool silent /* = false */) {
	if (newID < 0 || newID >= (int)Logger::entries.size()) {
		return;
	}
	auto entry = Logger::entries[newID];
	auto loggerVerbosity = getVerbosity(entry.getLogger());
	if (entry.getLevel() >= loggerVerbosity && loggerVerbosity != Logger::Level::DISABLED) {
		entryIDs.push_back(newID);
		if (!silent) {
			for (auto& out : outputs) {
				(*out) << entry.getOutput(showingTimestamps);
			}
		}
	}
}

void Logger::View::rescan() {
	entryIDs.clear();
	for (int i = 0; i < (int)Logger::entries.size(); i++) {
		notify(i, true);
	}
}

void Logger::View::setVerbosity(std::string loggerName, Logger::Level newLevel) {
	levels[loggerName] = newLevel;
	rescan();
}

Logger::Level Logger::View::getVerbosity(Logger* logger) {
	return getVerbosity(logger->getName());
}

Logger::Level Logger::View::getVerbosity(std::string loggerName) {
	if (levels.count(loggerName) == 0) {
		return defaultVerbosity;
	}
	return levels[loggerName];
}

void Logger::View::addOutputStream(std::ostream* out) {
	outputs.push_back(out);
}

void Logger::View::addOutputStream(std::ostream& out) {
	addOutputStream(&out);
}

void Logger::View::delOutputStream(std::ostream* out) {
	outputs.erase(std::remove(outputs.begin(), outputs.end(), out), outputs.end());
}

void Logger::View::delOutputStream(std::ostream& out) {
	delOutputStream(&out);
}

}
