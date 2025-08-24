#include "Commissar.h"

#include <array>
#include <complex>
#include <iostream>

#include <Debug.h>
#include <JSONReader.h>

Commissar::Commissar(const std::string& configFilepath, Wolf::ResourceNonOwner<TrayManager> tray) : m_tray(tray)
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

void Commissar::run()
{
    while (!m_stopRequested)
    {
        m_isSleeping = false;

        Wolf::Debug::sendInfo("----- Starting checks -----");

        std::vector<Project*> projectsToUpdate;
        checkForUpdates(projectsToUpdate);

        if (projectsToUpdate.empty())
        {
            m_tray->setState(APP_STATE::ALL_GOOD);
        }

        for (Project* project : projectsToUpdate)
        {
            if (project->executeScenario())
            {
                Project::MergeResult result = project->merge();

                if (result != Project::MergeResult::SUCCESS && result != Project::MergeResult::ALREADY_UP_TO_DATE)
                {
                    Wolf::Debug::sendError("----- Merge failed ! -----");

                    m_tray->setState(APP_STATE::IN_ERROR);

                    std::string unused;
                    std::cin >> unused;
                }
                else if (result == Project::MergeResult::SUCCESS)
                {
                    Wolf::Debug::sendInfo("----- Merge successful -----");

                    m_tray->setState(APP_STATE::ALL_GOOD);
                }
                else
                {
                    Wolf::Debug::sendInfo("----- Already up to date, no merge has been performed -----");

                    m_tray->setState(APP_STATE::ALL_GOOD);
                }
            }
            else
            {
                Wolf::Debug::sendError("----- Scenario failed ! -----");

                m_tray->setState(APP_STATE::IN_ERROR);

                std::string unused;
                std::cin >> unused;
            }
        }

        if (!m_stopRequested)
        {
            m_isSleeping = true;

            Wolf::Debug::sendInfo("----- No more jobs to do, starting a 10 minutes nap -----");
            std::this_thread::sleep_for(std::chrono::minutes(10));
        }
    }
}

void Commissar::stop()
{
    m_stopRequested = true;
}

bool Commissar::isSleeping() const
{
    return m_isSleeping;
}

void Commissar::checkForUpdates(std::vector<Project*>& outProjects)
{
    for (Project& project : m_projects)
    {
        Wolf::Debug::sendInfo("----- Checking " + project.getName() + " -----");
        if (project.isOutOfDate())
        {
            Wolf::Debug::sendInfo("----- Project is out of date -----");
            outProjects.push_back(&project);
        }
        else
        {
            Wolf::Debug::sendInfo("----- Project is up to date -----");
        }
    }
}
