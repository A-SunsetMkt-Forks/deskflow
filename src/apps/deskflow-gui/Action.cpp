/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2008 Volker Lanz (vl@fidra.de)
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Action.h"

#include <QSettings>
#include <QTextStream>

const char *Action::m_ActionTypeNames[] = {
    "keyDown",       "keyUp",     "keystroke", "switchToScreen", "switchInDirection", "lockCursorToScreen",
    "restartServer", "mouseDown", "mouseUp",   "mousebutton"
};

const char *Action::m_SwitchDirectionNames[] = {"left", "right", "up", "down"};
const char *Action::m_LockCursorModeNames[] = {"toggle", "on", "off"};

QString Action::text() const
{
  QString text = QString(m_ActionTypeNames[keySequence().isMouseButton() ? type() + 6 : type()]) + "(";

  switch (type()) {
  case keyDown:
  case keyUp:
  case keystroke: {
    text += keySequence().toString();

    if (!keySequence().isMouseButton()) {
      const QStringList &screens = typeScreenNames();
      if (haveScreens() && !screens.isEmpty()) {
        text += ",";

        for (int i = 0; i < screens.size(); i++) {
          text += screens[i];
          if (i != screens.size() - 1)
            text += ":";
        }
      } else
        text += ",*";
    }
  } break;

  case switchToScreen:
    text += switchScreenName();
    break;

  case switchInDirection:
    text += m_SwitchDirectionNames[m_SwitchDirection];
    break;

  case lockCursorToScreen:
    text += m_LockCursorModeNames[m_LockCursorMode];
    break;

  case restartAllConnections:
    text += "restart";
    break;
  default:
    Q_ASSERT(0);
    break;
  }

  text += ")";

  return text;
}

void Action::loadSettings(QSettings &settings)
{
  keySequence().loadSettings(settings);
  setType(settings.value("type", keyDown).toInt());

  typeScreenNames().clear();
  int numTypeScreens = settings.beginReadArray("typeScreenNames");
  for (int i = 0; i < numTypeScreens; i++) {
    settings.setArrayIndex(i);
    typeScreenNames().append(settings.value("typeScreenName").toString());
  }
  settings.endArray();

  setSwitchScreenName(settings.value("switchScreenName").toString());
  setSwitchDirection(settings.value("switchInDirection", switchLeft).toInt());
  setLockCursorMode(settings.value("lockCursorToScreen", lockCursorToggle).toInt());
  setActiveOnRelease(settings.value("activeOnRelease", false).toBool());
  setHaveScreens(settings.value("hasScreens", false).toBool());
  setRestartServer(settings.value("restartServer", false).toBool());
}

void Action::saveSettings(QSettings &settings) const
{
  keySequence().saveSettings(settings);
  settings.setValue("type", type());

  settings.beginWriteArray("typeScreenNames");
  for (int i = 0; i < typeScreenNames().size(); i++) {
    settings.setArrayIndex(i);
    settings.setValue("typeScreenName", typeScreenNames()[i]);
  }
  settings.endArray();

  settings.setValue("switchScreenName", switchScreenName());
  settings.setValue("switchInDirection", switchDirection());
  settings.setValue("lockCursorToScreen", lockCursorMode());
  settings.setValue("activeOnRelease", activeOnRelease());
  settings.setValue("hasScreens", haveScreens());
  settings.setValue("restartServer", restartServer());
}

bool Action::operator==(const Action &a) const
{
  return m_KeySequence == a.m_KeySequence && m_Type == a.m_Type && m_TypeScreenNames == a.m_TypeScreenNames &&
         m_SwitchScreenName == a.m_SwitchScreenName && m_SwitchDirection == a.m_SwitchDirection &&
         m_LockCursorMode == a.m_LockCursorMode && m_ActiveOnRelease == a.m_ActiveOnRelease &&
         m_HasScreens == a.m_HasScreens && m_restartServer == a.m_restartServer;
}

QTextStream &operator<<(QTextStream &outStream, const Action &action)
{
  if (action.activeOnRelease())
    outStream << ";";

  outStream << action.text();

  return outStream;
}
