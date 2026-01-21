#include "TrayManager.h"

#include <filesystem>

TrayManager::TrayManager()
{
}

void TrayManager::init()
{
    std::string iconPath = "icons/updatingIcon.ico";
#ifdef __linux__
    iconPath = std::filesystem::absolute(iconPath).string();
#endif

    m_tray.reset(new Tray::Tray("The Commit Commissar", iconPath));
    m_currentState = APP_STATE::COMMISSAR_WORKING;

    m_tray->addEntry(Tray::Button("Exit", [&] { m_tray->exit(); }));
}

void TrayManager::run()
{
    m_tray->run();
}

void TrayManager::setState(APP_STATE state)
{
    if (state != m_currentState)
    {
        m_currentState = state;
        updateIcon();
    }
}

void TrayManager::updateIcon()
{
    std::string iconPath = "icons/updatingIcon.ico";
    if (m_currentState == APP_STATE::ALL_GOOD)
    {
        iconPath = "icons/allGood.ico";
    }
    else if (m_currentState == APP_STATE::IN_ERROR)
    {
        iconPath = "icons/error.ico";
    }

#ifdef __linux__
    iconPath = std::filesystem::absolute(iconPath).string();
#endif

    m_tray->updateIcon(iconPath);
}
