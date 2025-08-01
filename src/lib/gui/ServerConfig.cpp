/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 Symless Ltd.
 * SPDX-FileCopyrightText: (C) 2008 Volker Lanz <vl@fidra.de>
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ServerConfig.h"

#include "Hotkey.h"
#include "MainWindow.h"
#include "common/Settings.h"
#include "dialogs/AddClientDialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QPushButton>

using enum ScreenConfig::Modifier;
using enum ScreenConfig::SwitchCorner;
using enum ScreenConfig::Fix;

static const struct
{
  int x;
  int y;
  const char *name;
} neighbourDirs[] = {
    {1, 0, "right"},
    {-1, 0, "left"},
    {0, -1, "up"},
    {0, 1, "down"},

};

const int serverDefaultIndex = 7;

ServerConfig::ServerConfig(MainWindow &mainWindow, int columns, int rows)
    : m_pMainWindow(&mainWindow),
      m_Screens(columns),
      m_Columns(columns),
      m_Rows(rows)
{
  recall();
}

bool ServerConfig::save(const QString &fileName) const
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  save(file);
  file.close();

  return true;
}

bool ServerConfig::operator==(const ServerConfig &sc) const
{
  return m_Screens == sc.m_Screens &&                           //
         m_Columns == sc.m_Columns &&                           //
         m_Rows == sc.m_Rows &&                                 //
         m_HasHeartbeat == sc.m_HasHeartbeat &&                 //
         m_Heartbeat == sc.m_Heartbeat &&                       //
         m_Protocol == sc.m_Protocol &&                         //
         m_RelativeMouseMoves == sc.m_RelativeMouseMoves &&     //
         m_Win32KeepForeground == sc.m_Win32KeepForeground &&   //
         m_HasSwitchDelay == sc.m_HasSwitchDelay &&             //
         m_SwitchDelay == sc.m_SwitchDelay &&                   //
         m_HasSwitchDoubleTap == sc.m_HasSwitchDoubleTap &&     //
         m_SwitchDoubleTap == sc.m_SwitchDoubleTap &&           //
         m_SwitchCornerSize == sc.m_SwitchCornerSize &&         //
         m_SwitchCorners == sc.m_SwitchCorners &&               //
         m_Hotkeys == sc.m_Hotkeys &&                           //
         m_DisableLockToScreen == sc.m_DisableLockToScreen &&   //
         m_ClipboardSharing == sc.m_ClipboardSharing &&         //
         m_ClipboardSharingSize == sc.m_ClipboardSharingSize && //
         m_pMainWindow == sc.m_pMainWindow;
}

void ServerConfig::save(QFile &file) const
{
  QTextStream outStream(&file);
  outStream << *this;
}

void ServerConfig::setupScreens()
{
  switchCorners().clear();
  screens().clear();
  hotkeys().clear();

  // m_NumSwitchCorners is used as a fixed size array. See Screen::init()
  for (int i = 0; i < static_cast<int>(NumSwitchCorners); i++)
    switchCorners() << false;

  // There must always be screen objects for each cell in the screens QList.
  // Unused screens are identified by having an empty name.
  for (int i = 0; i < numColumns() * numRows(); i++)
    addScreen(Screen());
}

void ServerConfig::commit()
{
  qDebug("committing server config");

  settings().beginGroup("internalConfig");
  settings().remove("");

  settings().setValue("numColumns", numColumns());
  settings().setValue("numRows", numRows());

  settings().setValue("hasHeartbeat", hasHeartbeat());
  settings().setValue("heartbeat", heartbeat());
  settings().setValue("protocol", static_cast<int>(protocol()));
  settings().setValue("relativeMouseMoves", relativeMouseMoves());
  settings().setValue("win32KeepForeground", win32KeepForeground());
  settings().setValue("hasSwitchDelay", hasSwitchDelay());
  settings().setValue("switchDelay", switchDelay());
  settings().setValue("hasSwitchDoubleTap", hasSwitchDoubleTap());
  settings().setValue("switchDoubleTap", switchDoubleTap());
  settings().setValue("switchCornerSize", switchCornerSize());
  settings().setValue("disableLockToScreen", disableLockToScreen());
  settings().setValue("clipboardSharing", clipboardSharing());
  settings().setValue("clipboardSharingSize", QVariant::fromValue(clipboardSharingSize()));

  writeSettings(settings(), switchCorners(), "switchCorner");

  settings().beginWriteArray("screens");
  for (int i = 0; i < screens().size(); i++) {
    settings().setArrayIndex(i);
    const auto &screen = screens()[i];
    screen.saveSettings(settings());
    auto screenName = Settings::value(Settings::Core::ScreenName).toString();
    if (screen.isServer() && screenName != screen.name()) {
      Settings::setValue(Settings::Core::ScreenName, screen.name());
    }
  }
  settings().endArray();

  settings().beginWriteArray("hotkeys");
  for (int i = 0; i < hotkeys().size(); i++) {
    settings().setArrayIndex(i);
    hotkeys()[i].saveSettings(settings().get());
  }
  settings().endArray();

  settings().endGroup();
}

void ServerConfig::recall()
{
  qDebug("recalling server config");

  settings().beginGroup("internalConfig");

  setNumColumns(settings().value("numColumns", 5).toInt());
  setNumRows(settings().value("numRows", 3).toInt());

  // we need to know the number of columns and rows before we can set up
  // ourselves
  setupScreens();

  haveHeartbeat(settings().value("hasHeartbeat", false).toBool());
  setHeartbeat(settings().value("heartbeat", 5000).toInt());
  setProtocol(static_cast<NetworkProtocol>(settings().value("protocol", static_cast<int>(protocol())).toInt()));
  setRelativeMouseMoves(settings().value("relativeMouseMoves", false).toBool());
  setWin32KeepForeground(settings().value("win32KeepForeground", false).toBool());
  haveSwitchDelay(settings().value("hasSwitchDelay", false).toBool());
  setSwitchDelay(settings().value("switchDelay", 250).toInt());
  haveSwitchDoubleTap(settings().value("hasSwitchDoubleTap", false).toBool());
  setSwitchDoubleTap(settings().value("switchDoubleTap", 250).toInt());
  setSwitchCornerSize(settings().value("switchCornerSize").toInt());
  setDisableLockToScreen(settings().value("disableLockToScreen", false).toBool());
  setClipboardSharingSize(
      settings().value("clipboardSharingSize", (int)ServerConfig::defaultClipboardSharingSize()).toULongLong()
  );
  setClipboardSharing(settings().value("clipboardSharing", true).toBool());

  readSettings(settings(), switchCorners(), "switchCorner", false, static_cast<int>(NumSwitchCorners));

  int numScreens = settings().beginReadArray("screens");
  Q_ASSERT(numScreens <= screens().size());
  for (int i = 0; i < numScreens; i++) {
    settings().setArrayIndex(i);
    screens()[i].loadSettings(settings());
    if (getServerName() == screens()[i].name()) {
      screens()[i].markAsServer();
    }
  }
  settings().endArray();

  int numHotkeys = settings().beginReadArray("hotkeys");
  for (int i = 0; i < numHotkeys; i++) {
    settings().setArrayIndex(i);
    Hotkey h;
    h.loadSettings(settings().get());
    hotkeys().append(h);
  }
  settings().endArray();

  settings().endGroup();
}

int ServerConfig::adjacentScreenIndex(int idx, int deltaColumn, int deltaRow) const
{
  if (screens()[idx].isNull())
    return -1;

  // if we're at the left or right end of the table, don't find results going
  // further left or right
  if ((deltaColumn > 0 && (idx + 1) % numColumns() == 0) || (deltaColumn < 0 && idx % numColumns() == 0))
    return -1;

  int arrayPos = idx + deltaColumn + deltaRow * numColumns();

  if (arrayPos >= screens().size() || arrayPos < 0)
    return -1;

  return arrayPos;
}

QTextStream &operator<<(QTextStream &outStream, const ServerConfig &config)
{
  using enum NetworkProtocol;

  outStream << "section: screens" << Qt::endl;

  for (const Screen &s : config.screens()) {
    if (!s.isNull())
      outStream << s.screensSection();
  }

  outStream << "end" << Qt::endl << Qt::endl;

  outStream << "section: aliases" << Qt::endl;

  for (const Screen &s : config.screens()) {
    if (!s.isNull())
      outStream << s.aliasesSection();
  }

  outStream << "end" << Qt::endl << Qt::endl;

  outStream << "section: links" << Qt::endl;

  for (int i = 0; i < config.screens().size(); i++) {
    if (!config.screens()[i].isNull()) {
      outStream << "\t" << config.screens()[i].name() << ":" << Qt::endl;
      for (unsigned int j = 0; j < std::size(neighbourDirs); j++) {
        int idx = config.adjacentScreenIndex(i, neighbourDirs[j].x, neighbourDirs[j].y);
        if (idx != -1 && !config.screens()[idx].isNull())
          outStream << "\t\t" << neighbourDirs[j].name << " = " << config.screens()[idx].name() << Qt::endl;
      }
    }
  }

  outStream << "end" << Qt::endl << Qt::endl;

  outStream << "section: options" << Qt::endl;

  if (config.hasHeartbeat())
    outStream << "\t" << "heartbeat = " << config.heartbeat() << Qt::endl;

  if (config.protocol() == Synergy) {
    outStream << "\t" << "protocol = synergy" << Qt::endl;
  } else if (config.protocol() == Barrier) {
    outStream << "\t" << "protocol = barrier" << Qt::endl;
  } else {
    qFatal("unrecognized protocol when writing config");
  }

  outStream << "\t"
            << "relativeMouseMoves = " << (config.relativeMouseMoves() ? "true" : "false") << Qt::endl;
  outStream << "\t"
            << "win32KeepForeground = " << (config.win32KeepForeground() ? "true" : "false") << Qt::endl;
  outStream << "\t"
            << "disableLockToScreen = " << (config.disableLockToScreen() ? "true" : "false") << Qt::endl;
  outStream << "\t"
            << "clipboardSharing = " << (config.clipboardSharing() ? "true" : "false") << Qt::endl;
  outStream << "\t"
            << "clipboardSharingSize = " << config.clipboardSharingSize() << Qt::endl;

  if (config.hasSwitchDelay())
    outStream << "\t"
              << "switchDelay = " << config.switchDelay() << Qt::endl;

  if (config.hasSwitchDoubleTap())
    outStream << "\t"
              << "switchDoubleTap = " << config.switchDoubleTap() << Qt::endl;

  outStream << "\t"
            << "switchCorners = none ";
  for (int i = 0; i < config.switchCorners().size(); i++)
    if (config.switchCorners()[i])
      outStream << "+" << config.switchCornerName(i) << " ";
  outStream << Qt::endl;

  outStream << "\t"
            << "switchCornerSize = " << config.switchCornerSize() << Qt::endl;

  for (const Hotkey &hotkey : config.hotkeys())
    outStream << hotkey;

  outStream << "end" << Qt::endl << Qt::endl;

  return outStream;
}

int ServerConfig::numScreens() const
{
  int rval = 0;

  for (const Screen &s : screens()) {
    if (!s.isNull())
      rval++;
  }

  return rval;
}

ScreenAddResults ServerConfig::autoAddScreen(const QString name)
{
  using enum AddAction;
  using enum ScreenAddResults;

  int serverIndex = -1;
  int targetIndex = -1;
  const auto screenName = Settings::value(Settings::Core::ScreenName).toString();
  if (!findScreenName(screenName, serverIndex) && !fixNoServer(screenName, serverIndex)) {
    return AutoAddScreenManualServer;
  }

  if (findScreenName(name, targetIndex)) {
    qDebug("ignoring screen already in config: %s", qPrintable(name));
    return AutoAddScreenIgnore;
  }

  auto result = static_cast<AddAction>(showAddClientDialog(name));

  if (result == AddClientIgnore) {
    return AutoAddScreenIgnore;
  }

  if (result == AddClientOther) {
    addToFirstEmptyGrid(name);
    return AutoAddScreenManualClient;
  }

  bool success = false;
  int startIndex = serverIndex;
  int offset = 1;
  int dirIndex = 0;

  if (result == AddClientLeft) {
    offset = -1;
    dirIndex = 1;
  } else if (result == AddClientUp) {
    offset = -5;
    dirIndex = 2;
  } else if (result == AddClientDown) {
    offset = 5;
    dirIndex = 3;
  }

  int idx = adjacentScreenIndex(startIndex, neighbourDirs[dirIndex].x, neighbourDirs[dirIndex].y);
  while (idx != -1) {
    if (screens()[idx].isNull()) {
      m_Screens[idx].setName(name);
      success = true;
      break;
    }

    startIndex += offset;
    idx = adjacentScreenIndex(startIndex, neighbourDirs[dirIndex].x, neighbourDirs[dirIndex].y);
  }

  if (!success) {
    addToFirstEmptyGrid(name);
    return AutoAddScreenManualClient;
  }

  return AutoAddScreenOk;
}

const QString ServerConfig::getServerName() const
{
  return Settings::value(Settings::Core::ScreenName).toString();
}

void ServerConfig::updateServerName()
{
  for (auto &screen : screens()) {
    if (screen.isServer()) {
      screen.setName(Settings::value(Settings::Core::ScreenName).toString());
      break;
    }
  }
}

const QString ServerConfig::configFile() const
{
  return Settings::value(Settings::Server::ExternalConfigFile).toString();
}

bool ServerConfig::useExternalConfig() const
{
  return Settings::value(Settings::Server::ExternalConfig).toBool();
}

bool ServerConfig::isFull() const
{
  bool isFull = true;

  for (const auto &screen : screens()) {
    if (screen.isNull()) {
      isFull = false;
      break;
    }
  }

  return isFull;
}

bool ServerConfig::screenExists(const QString &screenName) const
{
  bool isExists = false;

  for (const auto &screen : screens()) {
    if (!screen.isNull() && screen.name() == screenName) {
      isExists = true;
      break;
    }
  }

  return isExists;
}

void ServerConfig::addClient(const QString &clientName)
{
  int serverIndex = -1;
  const auto screenName = Settings::value(Settings::Core::ScreenName).toString();

  if (findScreenName(screenName, serverIndex)) {
    m_Screens[serverIndex].markAsServer();
  } else {
    fixNoServer(screenName, serverIndex);
  }

  m_Screens.addScreenByPriority(Screen(clientName));
}

void ServerConfig::setConfigFile(const QString &configFile) const
{
  Settings::setValue(Settings::Server::ExternalConfigFile, configFile);
}

void ServerConfig::setUseExternalConfig(bool useExternalConfig) const
{
  Settings::setValue(Settings::Server::ExternalConfig, useExternalConfig);
}

bool ServerConfig::findScreenName(const QString &name, int &index)
{
  bool found = false;
  for (int i = 0; i < screens().size(); i++) {
    if (!screens()[i].isNull() && screens()[i].name().compare(name) == 0) {
      index = i;
      found = true;
      break;
    }
  }
  return found;
}

bool ServerConfig::fixNoServer(const QString &name, int &index)
{
  bool fixed = false;
  if (screens()[serverDefaultIndex].isNull()) {
    m_Screens[serverDefaultIndex].setName(name);
    m_Screens[serverDefaultIndex].markAsServer();
    index = serverDefaultIndex;
    fixed = true;
  }

  return fixed;
}

int ServerConfig::showAddClientDialog(const QString &clientName)
{
  auto result = static_cast<int>(AddAction::AddClientIgnore);

  if (!m_pMainWindow->isActiveWindow()) {
    m_pMainWindow->showNormal();
    m_pMainWindow->activateWindow();
  }

  AddClientDialog addClientDialog(clientName, m_pMainWindow);
  addClientDialog.exec();
  result = static_cast<int>(addClientDialog.addResult());

  return result;
}

void ServerConfig::addToFirstEmptyGrid(const QString &clientName)
{
  for (int i = 0; i < screens().size(); i++) {
    if (screens()[i].isNull()) {
      m_Screens[i].setName(clientName);
      break;
    }
  }
}

size_t ServerConfig::defaultClipboardSharingSize()
{
  return 3 * 1024; // 3 MiB
}

size_t ServerConfig::setClipboardSharingSize(size_t size)
{
  if (size) {
    size += 512; // Round up to the nearest megabyte
    size /= 1024;
    size *= 1024;
    setClipboardSharing(true);
  } else {
    setClipboardSharing(false);
  }
  using std::swap;
  swap(size, m_ClipboardSharingSize);
  return size;
}

QSettingsProxy &ServerConfig::settings()
{
  return Settings::proxy();
}
