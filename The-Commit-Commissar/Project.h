#pragma once

#include <string>

#include <JSONReader.h>
#include <ResourceUniqueOwner.h>

#include "Scenario.h"

class Project
{
public:
    Project(const std::string& name, const std::string& repoPath, const std::string& username, const std::string& sourceBranchName, const std::string& targetBranchName, Wolf::JSONReader::JSONObjectInterface* scenarioObject,
        const std::function<void()>& reloadOutputJSONCallback);

    [[nodiscard]] bool isOutOfDate();
    [[nodiscard]] bool executeScenario();

    enum class MergeResult { ALREADY_UP_TO_DATE, INTERNAL_FAILURE, MERGE_FAILURE, PUSH_FAILURE, SET_ORIGIN_FAILURE, SUCCESS };
    [[nodiscard]] MergeResult merge();

    [[nodiscard]] const std::string& getName() { return m_name; };
    [[nodiscard]] bool isInError() const { return m_isInError;}

    void outputJSON(std::ofstream& outJSON);

private:
    std::string createLocalRepo(const std::string& branchName) const;
    void setUpdatedCheck();

    std::string m_name;
    std::string m_repoURL;
    std::string m_username;
    std::string m_sourceBranchName;
    std::string m_targetBranchName;

    std::string m_cloneFolder;
    uint64_t m_lastUpdatedTimestamp;
    Wolf::ResourceUniqueOwner<Scenario> m_scenario;

    bool m_isInError = false;
};
