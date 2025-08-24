#pragma once

#include <array>
#include <Debug.h>
#include <string>

static int executeCommandWithLogs(const std::string& command)
{
    Wolf::Debug::sendInfo("Executing command: " + command);

    std::array<char, 1024> buffer;
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
        Wolf::Debug::sendError("Can't execute command");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    Wolf::Debug::sendInfo("Command result: " + result);

    int returnCode = _pclose(pipe);
    Wolf::Debug::sendInfo("Command retuned: " + std::to_string(returnCode));

    return returnCode;
}
