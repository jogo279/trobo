#include "ABState.h"
#include <climits>
#include <cstddef>
#include <iostream> 

ABState::ABState(ABState * par) {
  aborted = false;
  parent = par;
}

int ABState::isAborted() const {
  return aborted || (parent && this->parent->isAborted());
}

void ABState::abort() {
  aborted = true;
}