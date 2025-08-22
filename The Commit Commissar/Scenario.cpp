#include "Scenario.h"

#include <Debug.h>

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
        std::string instructionValue = instructionObject->getPropertyString("value");

        if (instructionCommand == "executeWithResult")
        {
            std::string cmd = "cd " + m_cloneFolder + " && \"" + instructionValue + "\"";
            if (const size_t lastSlashPos = instructionValue.find_last_of("/"); lastSlashPos != std::string::npos)
            {
                cmd = "cd " + m_cloneFolder + " && cd \"" + instructionValue.substr(0, lastSlashPos) + "\" && \"" + instructionValue.substr(lastSlashPos + 1) + "\"";
            }

            int codeReturned = system(cmd.c_str());
            if (codeReturned != 0)
            {
                return false;
            }
        }
        else if (instructionCommand == "executePythonWithResult")
        {
            std::string cmd = "cd " + m_cloneFolder + " && python \"" + instructionValue + "\"";
            if (const size_t lastSlashPos = instructionValue.find_last_of("/"); lastSlashPos != std::string::npos)
            {
                cmd = "cd " + m_cloneFolder + " && cd \"" + instructionValue.substr(0, lastSlashPos) + "\" && python \"" + instructionValue.substr(lastSlashPos + 1) + "\"";
            }

            int codeReturned = system(cmd.c_str());
            if (codeReturned != 0)
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
