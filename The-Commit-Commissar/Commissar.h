#pragma once

#include <string>
#include <vector>

#include <ResourceUniqueOwner.h>

#include "Project.h"
#include "TrayManager.h"

class Commissar
{
public:
	Commissar(const std::string& configFilepath, Wolf::ResourceNonOwner<TrayManager> tray);

	void run();
	void stop();
	bool isSleeping() const;

private:
	void checkForUpdates(std::vector<Project*>& outProjects);
	void writeJSON();

	std::vector<Project> m_projects;
	Wolf::ResourceUniqueOwner<Wolf::JSONReader> m_configJSONReader;
	Wolf::ResourceNonOwner<TrayManager> m_tray;

	std::function<void()> m_reloadOutputJSONCallback;

	bool m_stopRequested = false;
	bool m_isSleeping = false;
};

