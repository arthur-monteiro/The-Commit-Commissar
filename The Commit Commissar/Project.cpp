#include "Project.h"

#include <array>
#include <fstream>

#include <Debug.h>
#include <filesystem>

#include "Helper.h"

Project::Project(const std::string& name, const std::string& repoURL, const std::string& sourceBranchName, const std::string& targetBranchName, Wolf::JSONReader::JSONObjectInterface* scenarioObject)
: m_name(name), m_repoURL(repoURL), m_sourceBranchName(sourceBranchName), m_targetBranchName(targetBranchName)
{
    m_cloneFolder = createLocalRepo(sourceBranchName);
    m_scenario.reset(new Scenario(scenarioObject, m_cloneFolder));
}

bool Project::isOutOfDate()
{
    std::string resetCmd = "cd " + m_cloneFolder + " && git reset --hard HEAD";
    system(resetCmd.c_str());

    std::string updateRepoCmd = "cd " + m_cloneFolder + " && git fetch && git pull";
    system(updateRepoCmd.c_str());

    std::string cmd = "cd " + m_cloneFolder + " & git log -1 --format=%at";

    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe)
        Wolf::Debug::sendError("Can't open " + std::string(cmd));

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    m_lastUpdatedTimestamp = std::stoul(result);

    std::string lastCheckFilePath = m_cloneFolder + "/last_commit_commissar_check.txt";
    std::ifstream localLastTimestampFileInput(lastCheckFilePath);
    if (!localLastTimestampFileInput.good())
    {
        std::ofstream localLastTimestampFileOutput(lastCheckFilePath);
        localLastTimestampFileOutput << "0";
        localLastTimestampFileOutput.close();

        localLastTimestampFileInput = std::ifstream(lastCheckFilePath);
    }
    std::string line;
    std::getline(localLastTimestampFileInput, line);
    uint64_t lastLocalRefreshTimestamp = -1;
    try
    {
        lastLocalRefreshTimestamp = std::stoul(line);
    } catch (const std::exception& e)
    {
        Wolf::Debug::sendError("Can't get uint64 from line " + line);
    }
    localLastTimestampFileInput.close();

    if (m_lastUpdatedTimestamp > lastLocalRefreshTimestamp)
    {
        return true;
    }

    return false;
}

bool Project::executeScenario()
{
    bool result = m_scenario->execute();
    if (!result)
    {
        m_isInError = true;
    }
    return result;
}

Project::MergeResult Project::merge()
{
    std::string targetCloneFolder = createLocalRepo(m_targetBranchName);
    std::string mergeCmd = "cd " + targetCloneFolder + " & git merge origin/" + m_sourceBranchName + " > mergeResult.txt";
    if (executeCommandWithLogs(mergeCmd) != 0)
    {
        m_isInError = true;
        return MergeResult::MERGE_FAILURE;
    }

    // Check if merge did something
    std::ifstream mergeResultFileInput(targetCloneFolder + "/mergeResult.txt");
    if (!mergeResultFileInput.good())
    {
        m_isInError = true;
        return MergeResult::INTERNAL_FAILURE;
    }
    std::string line;
    std::getline(mergeResultFileInput, line);
    mergeResultFileInput.close();

    if (line.contains("Already up to date."))
    {
        setUpdatedCheck();
        return MergeResult::ALREADY_UP_TO_DATE;
    }

    std::string pushCmd =  "cd " + targetCloneFolder + " & git push";
    if (executeCommandWithLogs(pushCmd) != 0)
    {
        m_isInError = true;
        return MergeResult::PUSH_FAILURE;
    }

    setUpdatedCheck();
    m_isInError = false;
    return MergeResult::SUCCESS;
}

std::string Project::createLocalRepo(const std::string& branchName) const
{
    std::string gitFolder = m_repoURL.substr(m_repoURL.find_last_of("/") + 1);
    std::string localCloneFolder = gitFolder + "_" + branchName;
    std::string outCloneFolder;

    if (std::filesystem::is_directory("Clones/" + localCloneFolder))
    {
        outCloneFolder = "Clones/" + localCloneFolder + "/" + gitFolder;
        std::string updateCmd = "cd " + outCloneFolder + " && git fetch && git pull";
        executeCommandWithLogs(updateCmd);
    }
    else
    {
        std::string createFolderCmd = "cd Clones & mkdir " + localCloneFolder;
        executeCommandWithLogs(createFolderCmd);

        outCloneFolder = "Clones/" + localCloneFolder;
        std::string cloneCmd = "cd " + outCloneFolder + " & git clone -b " + branchName + " " + m_repoURL;
        executeCommandWithLogs(cloneCmd);

        outCloneFolder += "/" + gitFolder;
    }

    return outCloneFolder;
}

void Project::setUpdatedCheck()
{
    std::string lastCheckFilePath = m_cloneFolder + "/last_commit_commissar_check.txt";
    std::ofstream localLastTimestampFileOutput(lastCheckFilePath);
    localLastTimestampFileOutput << m_lastUpdatedTimestamp;
    localLastTimestampFileOutput.close();
}
