#include "Scenario.h"

#include <Debug.h>
#include <filesystem>

#include "Helper.h"

Scenario::Scenario(Wolf::JSONReader::JSONObjectInterface* scenarioObject, const std::string& cloneFolder) : m_scenarioObject(scenarioObject), m_cloneFolder(cloneFolder)
{
}

bool Scenario::execute()
{
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

            executeCommandWithLogs(cmd.c_str());
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
                return false;
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
                return false;
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
            std::string copySource = m_cloneFolder + "/" + instructionObject->getPropertyString("source");
            std::string copyDestination = m_cloneFolder + "/" + instructionObject->getPropertyString("destination");

            std::filesystem::copy(copySource, copyDestination, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
        }
        else if (instructionCommand == "compareImages")
        {
            std::string image1 = m_cloneFolder + "/" + instructionObject->getPropertyString("image1");
            std::string image2 = m_cloneFolder + "/" + instructionObject->getPropertyString("image2");
            float tolerance = instructionObject->getPropertyFloat("tolerance");

            if (tolerance < compareImages(image1, image2))
            {
                return false;
            }
        }
        else
        {
            Wolf::Debug::sendError("Unknown instruction command");
            return false;
        }
    }

    return true;
}
