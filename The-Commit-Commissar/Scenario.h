#pragma once

#include <chrono>
#include <functional>

#include <JSONReader.h>

class Scenario
{
public:
    Scenario(Wolf::JSONReader::JSONObjectInterface* scenarioObject, const std::string& cloneFolder, const std::function<void()>& reloadOutputJSONCallback);

    bool execute();

    void outputStepsJSON(std::ofstream& outJSON, uint32_t tabCount) const;
    void outputHistoryJSON(std::ofstream& outJSON, uint32_t tabCount) const;

private:
    std::string computeAbsoluteOrRelativeFullPath(const std::string& pathStr);
    std::string computeAbsoluteFullPath(const std::string& pathStr) const;


    Wolf::JSONReader::JSONObjectInterface* m_scenarioObject;
    std::string m_cloneFolder;
    const std::function<void()>& m_reloadOutputJSONCallback;

    enum class Status { SUCCESS, FAILED, IN_PROGRESS, NOT_EXECUTED };

    static std::string computeStatusString(Status status);
    struct StepResult
    {
        std::string m_instructionCommand;

        Status m_status;
    };
    std::vector<StepResult> m_stepResults;

    struct HistoryResult
    {
        uint32_t m_durationInSeconds;
        Status m_status;
        std::chrono::system_clock::time_point m_endTime;
    };
    std::vector<HistoryResult> m_historyResults;
};
