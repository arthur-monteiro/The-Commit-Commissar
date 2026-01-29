#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

#include <Debug.h>
#include <Dump.h>
#include <ResourceUniqueOwner.h>

#include "Commissar.h"
#include "Dump.h"
#include "TrayManager.h"

Wolf::ResourceUniqueOwner<TrayManager> g_tray;
Wolf::ResourceUniqueOwner<Commissar> g_commissar;

#undef ERROR

void debugCallback(Wolf::Debug::Severity severity, Wolf::Debug::Type type, const std::string& message)
{
	std::ofstream logOutputFile("logs.txt", std::ios_base::app);

	std::chrono::time_point<std::chrono::system_clock> currentClock = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(currentClock);
	std::tm* now = std::localtime(&currentTime);
	logOutputFile << (now->tm_year + 1900) << '-'
		<< (now->tm_mon + 1) << '-'
		<<  now->tm_mday << ' '
		<<  now->tm_hour << ':'
		<< now->tm_min << ' ';

	switch (severity)
	{
		case Wolf::Debug::Severity::ERROR:
			logOutputFile << "Error : ";
			break;
		case Wolf::Debug::Severity::WARNING:
			logOutputFile << "Warning : ";
			break;
		case Wolf::Debug::Severity::INFO:
			logOutputFile << "Info : ";
			break;
		case Wolf::Debug::Severity::VERBOSE:
			return;
		default: ;
	}

	logOutputFile << message << '\n';
	logOutputFile.close();
}

void runTray()
{
	g_tray->init();
	g_tray->run();
}

void runCommissar()
{
	g_commissar->run();
}

int main()
{
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	SetUnhandledExceptionFilter(Wolf::unhandledExceptionFilter);
#endif

	Wolf::Debug::setCallback(debugCallback);

	g_tray.reset(new TrayManager());
	g_commissar.reset(new Commissar("config/config.json", g_tray.createNonOwnerResource()));

	std::thread runTrayThread(runTray);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::thread runCommissarThread(runCommissar);

	runTrayThread.join();
	g_commissar->stop();

	if (g_commissar->isSleeping())
	{
		runCommissarThread.detach();
	}
	else
	{
		runCommissarThread.join();
	}

	return EXIT_SUCCESS;
}