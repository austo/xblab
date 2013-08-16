#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "common/common.h" // rightnow

namespace xblab {

enum LogLevel {
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  DEBUG1,
  DEBUG2,
  DEBUG3,
  DEBUG4
};

template <typename T>
class Logger {
public:
  Logger();
  virtual ~Logger();
  std::ostringstream& get(LogLevel level = INFO);
  void setFile(const std::string& fname);
public:
  static LogLevel& reportingLevel();
  static std::string toString(LogLevel level);
  static LogLevel fromString(const std::string& level);
protected:
  std::ostringstream os;
private:
  Logger(const Logger&);
  Logger& operator =(const Logger&);
};

template <typename T>
Logger<T>::Logger() {}

template <typename T>
std::ostringstream&
Logger<T>::get(LogLevel level) {
  // tabs based on logging level
  os << rightnow() << "(" << toString(level) << ") - " <<
    std::string(level > DEBUG ? level - DEBUG : 0, '\t');
  return os;
}

template <typename T>
void
Logger<T>::setFile(const std::string& fname) {
  T::setFile(fname);
}

template <typename T>
Logger<T>::~Logger() {
  os << std::endl;
  T::output(os.str());
}

template <typename T>
LogLevel& 
Logger<T>::reportingLevel() {
  static LogLevel reportingLevel = DEBUG4;
  return reportingLevel;
}

template <typename T>
std::string 
Logger<T>::toString(LogLevel level) {
  static const char* const buffer[] = {
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "DEBUG1",
    "DEBUG2",
    "DEBUG3",
    "DEBUG4"
  };
  return buffer[level];
}

template <typename T>
LogLevel 
Logger<T>::fromString(const std::string& level) {
  if (level == "DEBUG4")
    return DEBUG4;
  if (level == "DEBUG3")
    return DEBUG3;
  if (level == "DEBUG2")
    return DEBUG2;
  if (level == "DEBUG1")
    return DEBUG1;
  if (level == "DEBUG")
    return DEBUG;
  if (level == "INFO")
    return INFO;
  if (level == "WARNING")
    return WARNING;
  if (level == "ERROR")
    return ERROR;
  Logger<T>().get(WARNING) << 
    "Unknown logging level '" << level << 
    "'. Using INFO level as default.";
  return INFO;
}

class LogOutput {
public:
  static FILE*& stream();
  static void setFile(const std::string& fname);
  static void output(const std::string& msg);
};

inline FILE*& LogOutput::stream() {
  static FILE* pStream = stderr;
  return pStream;
}

inline void LogOutput::setFile(const std::string& fname) {
  FILE* pFile = fopen(fname.c_str(), "a");
  if (pFile == NULL) {
    // leave as stderr
    return;
  }
  LogOutput::stream() = pFile;
}

inline void LogOutput::output(const std::string& msg) {   
  FILE* pStream = stream();
  if (!pStream) {
    return;
  }
  fprintf(pStream, "%s", msg.c_str());
  fflush(pStream);

  // if we're logging to a file instead of stderr, close it
  // if (!isatty(fileno(pStream))){
  //   fclose(pStream);
  // }
}

// Implementation
#define FILELOG_DECLSPEC

class FILELOG_DECLSPEC FileLogger : public Logger<LogOutput> {};
// typedef Logger<LogOutput> FILELog;

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL DEBUG4
#endif

// Only log messages under threshold
#define F_LOG(level)                                        \
  if (level > FILELOG_MAX_LEVEL) ;                          \
  else if (level > FileLogger::reportingLevel() ||          \
    !LogOutput::stream()) ;                                 \
  else FileLogger().get(level)


} // namespace xblab

#endif //LOGGER_H
