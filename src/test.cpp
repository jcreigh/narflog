/*----------------------------------------------------------------------------*/
/* Copyright (c) Creighton 2015. All Rights Reserved.                         */
/* Open Source Software - May be modified and shared but must                 */
/* be accompanied by the license file in the root source directory            */
/*----------------------------------------------------------------------------*/

//#include "narf/version.h"
#include "narf/Logger.h"
#include <gtest/gtest.h>

TEST(Logger, CreateParents) {
	auto loggerA = narf::Logger::getLogger("foo.bar.baz");
	auto loggerB = narf::Logger::getLogger("foo.bar");
	ASSERT_TRUE(loggerA->getParent() == loggerB);
	loggerA = narf::Logger::getLogger("foo");
	ASSERT_TRUE(loggerA == loggerB->getParent());
	loggerB = narf::Logger::getLogger();
	ASSERT_TRUE(loggerA->getParent() == loggerB);
}

TEST(Logger, getParent) {
	auto logger = narf::Logger::getLogger("foo.bar.baz");
	ASSERT_EQ(logger->getName(), "foo.bar.baz");
	ASSERT_EQ(logger->getParent()->getName(), "foo.bar");
	ASSERT_EQ(logger->getParent()->getParent()->getName(), "foo");
	ASSERT_EQ(logger->getParent()->getParent()->getParent()->getName(), "");
}

TEST(Logger, Output) {
	auto globalLogger = narf::Logger::getLogger();
	auto fooLogger = narf::Logger::getLogger("foo");
	auto globalView = new narf::Logger::View(narf::Logger::Level::TRACE, false);
	auto fooView = new narf::Logger::View(narf::Logger::Level::TRACE, false);
	fooView->setVerbosity("", narf::Logger::Level::DISABLED);
	std::stringstream globalStream;
	std::stringstream fooStream;
	globalView->addOutputStream(globalStream);
	fooLogger->debug("Hello, World");
	ASSERT_EQ("[DEBUG] [foo] Hello, World\n", globalStream.str());
	fooLogger->setVerbosity(narf::Logger::Level::WARN);
	fooLogger->info("Nothing");
	ASSERT_EQ("[DEBUG] [foo] Hello, World\n", globalStream.str());
	fooView->addOutputStream(fooStream);
	fooLogger->setVerbosity(narf::Logger::Level::INFO);
	fooLogger->error("Quack");
	ASSERT_EQ("[DEBUG] [foo] Hello, World\n[ERROR] [foo] Quack\n", globalStream.str());
	ASSERT_EQ("[ERROR] [foo] Quack\n", fooStream.str());
	globalLogger->fatal("Meow");
	ASSERT_EQ("[DEBUG] [foo] Hello, World\n[ERROR] [foo] Quack\n[FATAL] Meow\n", globalStream.str());
	ASSERT_EQ("[ERROR] [foo] Quack\n", fooStream.str());
	delete globalView;
	delete fooView;
}

TEST(Logger, StoredEntries) {
	auto logger = narf::Logger::getLogger("bar");
	auto view = new narf::Logger::View(narf::Logger::Level::DISABLED, false);
	view->setVerbosity("bar", narf::Logger::Level::TRACE);
	logger->fatal("Fatal entry");
	logger->debug("Debug entry");
	logger->trace("Trace entry");
	std::stringstream os;
	view->dumpEntries(os);
	ASSERT_EQ("[FATAL] [bar] Fatal entry\n[DEBUG] [bar] Debug entry\n[TRACE] [bar] Trace entry\n", os.str());
	os.str("");
	view->setVerbosity("bar", narf::Logger::Level::WARN);
	view->dumpEntries(os);
	//printf("|%s|\n", os.str().c_str());
	ASSERT_EQ("[FATAL] [bar] Fatal entry\n", os.str());
	delete view;
}


int main(int argc, char **argv)
{
	printf("NarfLog unit tests\n");
	//printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
