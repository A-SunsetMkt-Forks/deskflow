/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Symless Ltd.
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/LogOutputters.h"
#include "arch/Arch.h"
#include "base/Path.h"
#include "base/String.h"

#include <fstream>
#include <iostream>

constexpr auto s_logFileSizeLimit = 1024 * 1024; //!< Max Log size before rotating (1Mb)

//
// StopLogOutputter
//

void StopLogOutputter::open(const char *)
{
  // do nothing
}

void StopLogOutputter::close()
{
  // do nothing
}

void StopLogOutputter::show(bool)
{
  // do nothing
}

bool StopLogOutputter::write(LogLevel, const char *)
{
  return false;
}

//
// ConsoleLogOutputter
//

void ConsoleLogOutputter::open(const char *title)
{
  // do nothing
}

void ConsoleLogOutputter::close()
{
  // do nothing
}

void ConsoleLogOutputter::show(bool showIfEmpty)
{
  // do nothing
}

bool ConsoleLogOutputter::write(LogLevel level, const char *msg)
{
  if ((level >= LogLevel::Fatal) && (level <= LogLevel::Warning))
    std::cerr << msg << std::endl;
  else
    std::cout << msg << std::endl;
  std::cout.flush();
  return true;
}

void ConsoleLogOutputter::flush() const
{
  // do nothing
}

//
// SystemLogOutputter
//

void SystemLogOutputter::open(const char *title)
{
  ARCH->openLog(title);
}

void SystemLogOutputter::close()
{
  ARCH->closeLog();
}

void SystemLogOutputter::show(bool showIfEmpty)
{
  ARCH->showLog(showIfEmpty);
}

bool SystemLogOutputter::write(LogLevel level, const char *msg)
{
  ARCH->writeLog(level, msg);
  return true;
}

//
// SystemLogger
//

SystemLogger::SystemLogger(const char *title, bool blockConsole)
{
  // redirect log messages
  if (blockConsole) {
    m_stop = new StopLogOutputter; // NOSONAR - Adopted by `Log`
    CLOG->insert(m_stop);
  }
  m_syslog = new SystemLogOutputter; // NOSONAR - Adopted by `Log`
  m_syslog->open(title);
  CLOG->insert(m_syslog);
}

SystemLogger::~SystemLogger()
{
  CLOG->remove(m_syslog);
  delete m_syslog;
  if (m_stop != nullptr) {
    CLOG->remove(m_stop);
    delete m_stop;
  }
}

//
// FileLogOutputter
//

FileLogOutputter::FileLogOutputter(const char *logFile)
{
  setLogFilename(logFile);
}

void FileLogOutputter::setLogFilename(const char *logFile)
{
  assert(logFile != nullptr);
  m_fileName = logFile;
}

bool FileLogOutputter::write(LogLevel level, const char *message)
{
  bool moveFile = false;

  std::ofstream m_handle;
  m_handle.open(deskflow::filesystem::path(m_fileName), std::fstream::app);
  if (m_handle.is_open() && m_handle.fail() != true) {
    m_handle << message << std::endl;

    // when file size exceeds limits, move to 'old log' filename.
    size_t p = m_handle.tellp();
    if (p > s_logFileSizeLimit) {
      moveFile = true;
    }
  }
  m_handle.close();

  if (moveFile) {
    std::string oldLogFilename = deskflow::string::sprintf("%s.1", m_fileName.c_str());
    remove(oldLogFilename.c_str());
    rename(m_fileName.c_str(), oldLogFilename.c_str());
  }

  return true;
}

void FileLogOutputter::open(const char *title)
{
  // do nothing
}

void FileLogOutputter::close()
{
  // do nothing
}

void FileLogOutputter::show(bool showIfEmpty)
{
  // do nothing
}
