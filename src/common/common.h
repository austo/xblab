#ifndef XB_COMMON_H
#define XB_COMMON_H

#include <assert.h>

#include <string>
#include <vector>
#include <ctime>

/* Putting this here is a hack */
namespace xblab {

typedef unsigned short sched_t;


inline std::string rightnow() {
  static time_t now;
  static char buf[30];
  time(&now);
  struct tm *t_info = localtime(&now);
  strftime(buf, 30, "%F %T - ", t_info);
  return std::string(buf);
}


template <class T>
static std::vector<T>
vectorize_string(std::string& s) {
  return std::vector<T>((T*)&s[0], ((T*)&s[0] + s.size()));
}

} // namespace xblab
#endif