#include "Scenario.h"

#include <filesystem>
#include <fstream>
#include <thread>

#include <Debug.h>
#include <JSONReader.h>

#include "Helper.h"

Scenario::Scenario(Wolf::JSONReader::JSONObjectInterface* scenarioObject, const std::string& cloneFolder, const std::function<void()>& reloadOutputJSONCallback)
    : m_scenarioObject(scenarioObject), m_cloneFolder(cloneFolder), m_reloadOutputJSONCallback(reloadOutputJSONCallback)
{
    uint32_t instructionCount = m_scenarioObject->getArraySize("instructions");
    m_stepResults.resize(instructionCount);

    for (uint32_t i = 0; i < instructionCount; i++)
    {
        Wolf::JSONReader::JSONObjectInterface* instructionObject = m_scenarioObject->getArrayObjectItem("instructions", i);
        std::string instructionCommand = instructionObject->getPropertyString("command");

        StepResult& stepResult = m_stepResults[i];
        stepResult.m_instructionCommand = instructionCommand;
        stepResult.m_status = Status::NOT_EXECUTED;
    }

    m_reloadOutputJSONCallback();
}

bool Scenario::execute()
{
    auto startTimer = std::chrono::high_resolution_clock::now();

    std::vector<std::string> windowTitlesStarted;

    bool hadAnError = false;
    std::string errorMessage;
    uint32_t instructionCount = m_scenarioObject->getArraySize("instructions");

    for (uint32_t i = 0; i < instructionCount; i++)
    {
        StepResult& stepResult = m_stepResults[i];
        stepResult.m_status = Status::NOT_EXECUTED;
    }

    m_reloadOutputJSONCallback();

    for (uint32_t i = 0; i < instructionCount; i++)
    {
        Wolf::JSONReader::JSONObjectInterface* instructionObject = m_scenarioObject->getArrayObjectItem("instructions", i);
        std::string instructionCommand = instructionObject->getPropertyString("command");

        StepResult& stepResult = m_stepResults[i];
        stepResult.m_instructionCommand = instructionCommand;
        stepResult.m_status = Status::IN_PROGRESS;

        m_reloadOutputJSONCallback();

        if (instructionCommand == "executeWithoutResult" || instructionCommand == "executeWithResult")
        {
            std::string instructionValue = instructionObject->getPropertyString("value");

            std::filesystem::path clonePath(m_cloneFolder);
            std::filesystem::path exeRelativePath(instructionValue);
            std::filesystem::path absoluteExePath = std::filesystem::absolute(clonePath / exeRelativePath);
            std::string absoluteExePathStr = absoluteExePath.string();

            std::string cmd;
#ifdef _WIN32
            std::string workingDirectoryStr = absoluteExePath.parent_path().string();
            if (instructionObject->hasProperty("workingDirectory"))
            {
                std::filesystem::path workingDirectoryRelativePath(instructionObject->getPropertyString("workingDirectory"));
                std::filesystem::path workingDirectoryAbsolutePath = std::filesystem::absolute(clonePath / workingDirectoryRelativePath);
                workingDirectoryStr = workingDirectoryAbsolutePath.string();
            }
            cmd += "cd \"" + workingDirectoryStr + "\" && ";
            cmd += "call \"" + absoluteExePathStr + "\" ";

            if (instructionObject->hasProperty("commandLineArguments"))
            {
                cmd += " " + instructionObject->getPropertyString("commandLineArguments");
            }
#elif __linux__
            if (instructionObject->hasProperty("workingDirectory"))
            {
                std::string workingDirectory = instructionObject->getPropertyString("workingDirectory");
                cmd += "(cd \"" + workingDirectory + "\" && \"" + absoluteExePath + "\"";
            }
            else
            {
                cmd += "\"" + absoluteExePath + "\"";
            }

            if (instructionObject->hasProperty("commandLineArguments"))
            {
                cmd += " " + instructionObject->getPropertyString("commandLineArguments");
            }

            if (instructionObject->hasProperty("workingDirectory"))
            {
                cmd += ")";
            }
#endif

            int codeReturned = executeCommandWithLogs(cmd);

            if (instructionCommand == "executeWithResult")
            {
                if (codeReturned != 0)
                {
                    hadAnError = true;
                    errorMessage = instructionCommand + " failed: code returned = " + std::to_string(codeReturned);
                    Wolf::Debug::sendInfo(errorMessage);
                }
            }
        }
        else if (instructionCommand == "buildCMake")
        {
            std::string folder = instructionObject->getPropertyString("folder");
            std::string compiler = instructionObject->getPropertyString("compiler");
            std::string buildOptions = instructionObject->getPropertyString("buildOptions");

            std::string goToFolderCmd =  "cd " + m_cloneFolder + " ";
            if (!folder.empty())
            {
                goToFolderCmd += "&& cd \"" + folder + "\" ";
            }

            std::string generateCommand = goToFolderCmd + "&& cmake -S . -B build -G \"" + compiler + "\"" + " " + buildOptions;
            int codeReturned = executeCommandWithLogs(generateCommand);
            if (codeReturned != 0)
            {
                hadAnError = true;
                errorMessage = instructionCommand + " failed at generation: code returned = " + std::to_string(codeReturned);
            }

            std::string buildCommand = goToFolderCmd + "&& cmake --build build";
            codeReturned = executeCommandWithLogs(buildCommand);
            if (codeReturned != 0)
            {
                hadAnError = true;
                errorMessage = instructionCommand + " failed at build: code returned = " + std::to_string(codeReturned);
            }
        }
        else if (instructionCommand == "executePythonWithResult")
        {
            std::string instructionValue = instructionObject->getPropertyString("value");

            std::string cmd = "cd " + m_cloneFolder + " && python \"" + instructionValue + "\"";
            if (const size_t lastSlashPos = instructionValue.find_last_of("/"); lastSlashPos != std::string::npos)
            {
                cmd = "cd " + m_cloneFolder + " && cd \"" + instructionValue.substr(0, lastSlashPos) + "\" && python \"" + instructionValue.substr(lastSlashPos + 1) + "\"";
            }

            int codeReturned = executeCommandWithLogs(cmd);
            if (codeReturned != 0)
            {
                hadAnError = true;
                errorMessage = instructionCommand + " failed: code returned = " + std::to_string(codeReturned);
                Wolf::Debug::sendInfo(errorMessage);
            }
        }
        else if (instructionCommand == "copyFile")
        {
            std::string copySource = m_cloneFolder + "/" + instructionObject->getPropertyString("source");
            std::string copyDestination = m_cloneFolder + "/" + instructionObject->getPropertyString("destination");

            std::filesystem::copy(copySource, copyDestination, std::filesystem::copy_options::overwrite_existing);
        }
        else if (instructionCommand == "createFolder")
        {
            std::string value = instructionObject->getPropertyString("value");
            std::filesystem::create_directory(m_cloneFolder + "/" + value);
        }
        else if (instructionCommand == "copyFolder")
        {
            std::string copySource = computeAbsoluteOrRelativeFullPath(instructionObject->getPropertyString("source"));
            std::string copyDestination = m_cloneFolder + "/" + instructionObject->getPropertyString("destination");

            std::filesystem::copy(copySource, copyDestination, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
        }
        else if (instructionCommand == "compareImages")
        {
            std::string image1 = m_cloneFolder + "/" + instructionObject->getPropertyString("image1");
            std::string image2 = m_cloneFolder + "/" + instructionObject->getPropertyString("image2");
            float tolerance = instructionObject->getPropertyFloat("tolerance") + 0.0001f;

            float result = compareImages(image1, image2);
            if (tolerance < result)
            {
                hadAnError = true;
                errorMessage = "Image comparison failed: tolerance was " + std::to_string(tolerance) + " result is " + std::to_string(result);
                Wolf::Debug::sendInfo(errorMessage);
            }
        }
        else if (instructionCommand == "commandNoWait")
        {
            std::string windowTitle = "CommandNoWait_" + std::to_string(windowTitlesStarted.size());
            windowTitlesStarted.push_back(windowTitle);

            std::string instructionValue = "start \"" + windowTitle + "\" cmd /k \"" + instructionObject->getPropertyString("value") + "\"";
            system(instructionValue.c_str());
        }
        else if (instructionCommand == "wait")
        {
            std::this_thread::sleep_for(std::chrono::seconds(static_cast<uint32_t>(instructionObject->getPropertyFloat("timeInSeconds"))));
        }
        else if (instructionCommand == "androidInstallAPK")
        {
            std::string device = instructionObject->getPropertyString("device");
            std::string fullPathToAPK = computeAbsoluteFullPath(instructionObject->getPropertyString("apkPath"));
            std::string command = "cd %ANDROID_HOME%/platform-tools && adb -s " + device + " install \"" + fullPathToAPK + "\"";
            executeCommandWithLogs(command);
        }
        else if (instructionCommand == "androidStartApp")
        {
            std::string windowTitle = "AndroidStartApp_" + std::to_string(windowTitlesStarted.size());
            windowTitlesStarted.push_back(windowTitle);

            std::string device = instructionObject->getPropertyString("device");
            std::string appName = instructionObject->getPropertyString("appName");
            std::string command = "start \"" + windowTitle + "\" cmd /k \"cd %ANDROID_HOME%/platform-tools && adb -s " + device + " shell am start -n " + appName + "\"";
            system(command.c_str());
        }
        else if (instructionCommand == "screenshot")
        {
            std::string windowTitle = instructionObject->getPropertyString("windowTitle");
            std::string outputFolder = computeAbsoluteOrRelativeFullPath(instructionObject->getPropertyString("outputFolder"));
            std::string outputImageName = instructionObject->getPropertyString("outputImageName");
            uint32_t offsetX = static_cast<uint32_t>(instructionObject->getPropertyFloat("offsetX"));
            uint32_t offsetY = static_cast<uint32_t>(instructionObject->getPropertyFloat("offsetY"));
            uint32_t reduceX = static_cast<uint32_t>(instructionObject->getPropertyFloat("reduceX"));
            uint32_t reduceY = static_cast<uint32_t>(instructionObject->getPropertyFloat("reduceY"));

            std::filesystem::copy("screenshotTemplate.py", outputFolder + "/screenshotScript.py", std::filesystem::copy_options::overwrite_existing);

            std::ofstream scriptFile;
            scriptFile.open(outputFolder + "/screenshotScript.py", std::ios_base::app);
            scriptFile << "screenshot(\"" + windowTitle + "\", \"" + outputImageName + "\", " + std::to_string(offsetX) + ", " + std::to_string(offsetY) + ", " +
                std::to_string(reduceX) + ", " + std::to_string(reduceY) + ")";
            scriptFile.close();

            std::string command = "cd " + outputFolder + " && python screenshotScript.py";
            executeCommandWithLogs(command);
        }
        else
        {
            hadAnError = true;
            errorMessage = "Unknown instruction command";
            Wolf::Debug::sendError(errorMessage);
        }

        stepResult.m_status = hadAnError ? Status::FAILED : Status::SUCCESS;
        m_reloadOutputJSONCallback();

        if (hadAnError)
        {
            break;
        }
    }

    for (const std::string& windowTitle : windowTitlesStarted)
    {
        std::string killCmd = "taskkill /FI \"WindowTitle eq "  + windowTitle + "*\" /T /F";
        system(killCmd.c_str());
    }

    if (hadAnError)
    {
        Wolf::Debug::sendInfo("Senario failed: " + errorMessage);
    }

    auto endTimer = std::chrono::high_resolution_clock::now();

    HistoryResult& historyResult = m_historyResults.emplace_back();
    historyResult.m_durationInSeconds = std::chrono::duration_cast<std::chrono::seconds>(endTimer - startTimer).count();
    historyResult.m_endTime = std::chrono::system_clock::now();
    historyResult.m_status = hadAnError ? Status::FAILED : Status::SUCCESS;
    m_reloadOutputJSONCallback();

    return !hadAnError;
}

void Scenario::outputStepsJSON(std::ofstream& outJSON, uint32_t tabCount) const
{
    for (uint32_t stepIdx = 0; stepIdx < m_stepResults.size(); stepIdx++)
    {
        const StepResult& stepResult = m_stepResults[stepIdx];

        for (uint32_t i = 0; i < tabCount; i++)
            outJSON << "\t";

        outJSON << R"({ "instructionCommand": ")" + stepResult.m_instructionCommand + "\", ";


        outJSON << R"("status": ")" + computeStatusString(stepResult.m_status) + "\" }";

        if (stepIdx != m_stepResults.size() - 1)
        {
            outJSON << ",";
        }
        outJSON << "\n";
    }
}

std::string formatDuration(uint32_t totalSeconds)
{
    if (totalSeconds == 0) return "0s";

    uint32_t minutes = totalSeconds / 60;
    uint32_t seconds = totalSeconds % 60;

    std::string result = "";

    if (minutes > 0)
    {
        result += std::to_string(minutes) + "m ";
    }

    if (seconds > 0 || totalSeconds < 60)
    {
        result += std::to_string(seconds) + "s";
    }

    if (!result.empty() && result.back() == ' ')
    {
        result.pop_back();
    }

    return result;
}

void Scenario::outputHistoryJSON(std::ofstream& outJSON, uint32_t tabCount) const
{
    for (uint32_t historyIdx = 0; historyIdx < m_historyResults.size(); historyIdx++)
    {
        const HistoryResult& historyResult = m_historyResults[historyIdx];

        for (uint32_t i = 0; i < tabCount; i++)
            outJSON << "\t";

        outJSON << R"({ "time": ")" + std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(historyResult.m_endTime));
        outJSON << R"(", "duration": ")" + formatDuration(historyResult.m_durationInSeconds) + R"(", "result": ")" + computeStatusString(historyResult.m_status) + "\" }";

        if (historyIdx != m_historyResults.size() - 1)
            outJSON << ",";
        outJSON << "\n";
    }
}

std::string Scenario::computeAbsoluteOrRelativeFullPath(const std::string& pathStr)
{
    std::filesystem::path path(pathStr);
    if (path.is_absolute())
    {
        return pathStr;
    }
    else
    {
        return m_cloneFolder + "/" + pathStr;
    }
}

std::string Scenario::computeAbsoluteFullPath(const std::string& pathStr)
{
    std::filesystem::path path(pathStr);
    if (path.is_absolute())
    {
        return pathStr;
    }
    else
    {
        std::u8string currentPath(std::filesystem::current_path().u8string());
        return std::string(currentPath.begin(), currentPath.end()) + "/" + m_cloneFolder + "/" + pathStr;
    }
}

std::string Scenario::computeStatusString(Status status) const
{
    switch (status)
    {
        case Status::SUCCESS:
            return "success";
        case Status::FAILED:
            return "failed";
        case Status::IN_PROGRESS:
            return "in-progress";
        case Status::NOT_EXECUTED:
            return "notExecuted";
    }

    return "";
}
