#include "Scenario.h"

#include <Debug.h>
#include <JSONReader.h>
#include <filesystem>
#include <fstream>

#include "Helper.h"

Scenario::Scenario(Wolf::JSONReader::JSONObjectInterface* scenarioObject, const std::string& cloneFolder) : m_scenarioObject(scenarioObject), m_cloneFolder(cloneFolder)
{
}

bool Scenario::execute()
{
    std::vector<std::string> windowTitlesStarted;

    bool hadAnError = false;
    std::string errorMessage;
    uint32_t instructionCount = m_scenarioObject->getArraySize("instructions");
    for (uint32_t i = 0; i < instructionCount; i++)
    {
        Wolf::JSONReader::JSONObjectInterface* instructionObject = m_scenarioObject->getArrayObjectItem("instructions", i);
        std::string instructionCommand = instructionObject->getPropertyString("command");

        if (instructionCommand == "executeWithoutResult")
        {
            std::string instructionValue = instructionObject->getPropertyString("value");

            std::string cmd = "cd " + m_cloneFolder + " && " + instructionValue + "";
            if (const size_t lastSlashPos = instructionValue.find_last_of("/"); lastSlashPos != std::string::npos)
            {
                cmd = "cd " + m_cloneFolder + " && cd \"" + instructionValue.substr(0, lastSlashPos) + "\" && \"" + instructionValue.substr(lastSlashPos + 1) + "\"";
            }

            executeCommandWithLogs(cmd);
        }
        else if (instructionCommand == "executeWithResult")
        {
            std::string instructionValue = instructionObject->getPropertyString("value");

            std::string cmd = "cd " + m_cloneFolder + " && \"" + instructionValue + "\"";
            if (const size_t lastSlashPos = instructionValue.find_last_of("/"); lastSlashPos != std::string::npos)
            {
                cmd = "cd " + m_cloneFolder + " && cd \"" + instructionValue.substr(0, lastSlashPos) + "\" && \"" + instructionValue.substr(lastSlashPos + 1) + "\"";
            }

            int codeReturned = executeCommandWithLogs(cmd);
            if (codeReturned != 0)
            {
                hadAnError = true;
                errorMessage = instructionCommand + " failed: code returned = " + std::to_string(codeReturned);
                Wolf::Debug::sendInfo(errorMessage);
            }
        }
        else if (instructionCommand == "buildCMake")
        {
            std::string folder = instructionObject->getPropertyString("folder");
            std::string compiler = instructionObject->getPropertyString("compiler");
            std::string buildOptions = instructionObject->getPropertyString("buildOptions");

            std::string generateCommand = "cd " + m_cloneFolder + " && cmake -S . -B build -G \"" + compiler + "\"" + " " + buildOptions;
            int codeReturned = executeCommandWithLogs(generateCommand);
            if (codeReturned != 0)
            {
                hadAnError = true;
                errorMessage = instructionCommand + " failed at generation: code returned = " + std::to_string(codeReturned);
            }

            std::string buildCommand = "cd " + m_cloneFolder + " && cmake --build build";
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

    return !hadAnError;
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
