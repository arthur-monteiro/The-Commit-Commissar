#include "Commissar.h"

#include <array>
#include <complex>
#include <iostream>

#include <Debug.h>
#include <JSONReader.h>

Commissar::Commissar(const std::string& configFilepath)
{
    Wolf::JSONReader::FileReadInfo fileInfo(configFilepath);
    m_configJSONReader.reset(new Wolf::JSONReader(fileInfo));

    uint32_t projectCount = m_configJSONReader->getRoot()->getArraySize("projects");
    m_projects.reserve(projectCount);

    for (uint32_t i = 0; i < projectCount; i++)
    {
        Wolf::JSONReader::JSONObjectInterface* projectObject = m_configJSONReader->getRoot()->getArrayObjectItem("projects", i);

        m_projects.emplace_back(projectObject->getPropertyString("name"), projectObject->getPropertyString("repoURL"), projectObject->getPropertyString("sourceBranchName"),
            projectObject->getPropertyString("targetBranchName"), projectObject->getPropertyObject("scenario"));
    }
}

[[noreturn]] void Commissar::run()
{
    while (true)
    {
        std::cout << "----- Starting checks ----- \n\n";

        std::vector<Project*> projectsToUpdate;
        checkForUpdates(projectsToUpdate);

        for (Project* project : projectsToUpdate)
        {
            if (project->executeScenario())
            {
                Project::MergeResult result = project->merge();

                if (result != Project::MergeResult::SUCCESS && result != Project::MergeResult::ALREADY_UP_TO_DATE)
                {
                    std::cerr << "----- Merge failed ! -----" << std::endl;

                    std::string unused;
                    std::cin >> unused;
                }
                else if (result == Project::MergeResult::SUCCESS)
                {
                    std::cout << "----- Merge successful -----\n\n";
                }
                else
                {
                    std::cout << "----- Already up to date, no merge has been performed -----\n\n";
                }
            }
            else
            {
                std::cerr << "----- Scenario failed ! -----" << std::endl;

                std::string unused;
                std::cin >> unused;
            }
        }

        std::cout << "----- No more jobs to do, starting a 10 minutes nap ----- \n\n";
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

void Commissar::checkForUpdates(std::vector<Project*>& outProjects)
{
    for (Project& project : m_projects)
    {
        std::cout << "----- Checking " + project.getName() + " ----- \n";
        if (project.isOutOfDate())
        {
            std::cout << "----- Project is out of date ----- \n\n";
            outProjects.push_back(&project);
        }
        else
        {
            std::cout << "----- Project is up to date ----- \n\n";
        }
    }
}
