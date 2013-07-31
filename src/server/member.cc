#include <cstdio>

#include "member.h"
#include "memberBaton.h"

using namespace std;

namespace xblab {

void
Member::assume(const Member& other) {
  publicKey = other.publicKey;
  present = other.present;
}


void
Member::assume(Member* other) {
  publicKey = other->publicKey;
  present = other->present;
  delete other;
}


void
Member::notifyStartChat() {
  printf("%s starting chat\n", handle.c_str());
}

} // namespace xblab