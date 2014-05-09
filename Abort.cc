#include "Abort.h"

#include <cstddef>
#include <iostream> 
Abort::Abort(Abort * par) {
  count = 0;
  aborted = false;
  parent = par;
}

int Abort::isAborted() const {
  return aborted || (parent && this->parent->isAborted());
}

void Abort::abort() {
  aborted = true;
}