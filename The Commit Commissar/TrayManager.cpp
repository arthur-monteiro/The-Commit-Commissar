#include "TrayManager.h"

TrayManager::TrayManager()
{
}

void TrayManager::init()
{
    m_tray.reset(new Tray::Tray("The Commit Commissar", "icons/updatingIcon.ico"));
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

    m_tray->updateIcon(iconPath);
}
