#pragma once

#include <JSONReader.h>

class Scenario
{
public:
    Scenario(Wolf::JSONReader::JSONObjectInterface* scenarioObject, const std::string& cloneFolder);

    bool execute();

private:
    Wolf::JSONReader::JSONObjectInterface* m_scenarioObject;
    std::string m_cloneFolder;
};
