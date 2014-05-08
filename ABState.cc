#include "ABState.h"
#include <climits>
ABState::ABState() {
  a = INT_MIN;
  b = INT_MAX;
}

int ABState::getA() const {
  return a;
}

int ABState::getB() const {
  return b;
}
void ABState::setA(int val) {
  a = val;
}

void ABState::setB(int val) {
  b = val;
}
