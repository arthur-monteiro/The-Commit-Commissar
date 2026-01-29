#pragma once

#include <ResourceUniqueOwner.h>
#include <tray.hpp>

enum class APP_STATE
{
    COMMISSAR_WORKING,
    ALL_GOOD,
    IN_ERROR
};

class TrayManager
{
public:
    TrayManager();

    void init();
    void run();
    void setState(APP_STATE state);

private:
    void updateIcon();

    Wolf::ResourceUniqueOwner<Tray::Tray> m_tray;

    APP_STATE m_currentState;
};
