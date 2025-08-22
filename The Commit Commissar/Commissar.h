#pragma once

#include <string>
#include <vector>

#include <ResourceUniqueOwner.h>

#include "Project.h"

class Commissar
{
public:
	Commissar(const std::string& configFilepath);

	void run();

private:
	void checkForUpdates(std::vector<Project*>& outProjects);

	std::vector<Project> m_projects;
	Wolf::ResourceUniqueOwner<Wolf::JSONReader> m_configJSONReader;
};

