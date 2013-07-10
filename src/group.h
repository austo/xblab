#ifndef GROUP_H
#define GROUP_H

#include <string>

namespace xblab {

struct Group {
  Group(){}
  Group(int id, std::string name, std::string url) :
    id(id), name(name), url(url) { }
  int id;
  std::string name;
  std::string url;
};
} //namespace xblab


#endif