/*
    SPDX-FileCopyrightText: 2014 Till Theato <root@ttill.de>

    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

    This file is part of Kdenlive. See www.kdenlive.org.
*/

#include "jogmanager.h"
#include "core.h"
#include "jogaction.h"
#include "jogshuttle.h"
#include "jogshuttleconfig.h"
#include "kdenlivesettings.h"
#include "mainwindow.h"

JogManager::JogManager(QObject *parent)
    : QObject(parent)

{
    slotConfigurationChanged();

    connect(pCore->window(), &MainWindow::configurationChanged, this, &JogManager::slotConfigurationChanged);
}

void JogManager::slotConfigurationChanged()
{
    delete m_shuttleAction;
    m_shuttleAction = nullptr;
    delete m_shuttle;
    m_shuttle = nullptr;

    if (KdenliveSettings::enableshuttle()) {
        m_shuttle = new JogShuttle(JogShuttle::canonicalDevice(KdenliveSettings::shuttledevice()));
        m_shuttleAction = new JogShuttleAction(m_shuttle, JogShuttleConfig::actionMap(KdenliveSettings::shuttlebuttons()));

        connect(m_shuttleAction, &JogShuttleAction::action, this, &JogManager::slotDoAction);
    }
}

void JogManager::slotDoAction(const QString &actionName)
{
    QAction *action = pCore->window()->actionCollection()->action(actionName);
    if (!action) {
        fprintf(stderr, "%s", QStringLiteral("shuttle action '%1' unknown\n").arg(actionName).toLatin1().constData());
        return;
    }
    action->trigger();
}
