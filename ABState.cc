#include "ABState.h"
#include <climits>
#include <cstddef>
#include <iostream> 

#define MAX(a,b) ((a)>(b))?(a):(b)
#define MIN(a,b) ((a)<(b))?(a):(b)

ABState::ABState(ABState * par) {
  count = 0;
  aborted = false;
  parent = par;
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


int ABState::isAborted() const {
  return aborted || (parent && this->parent->isAborted());
}

std::tuple<int,int,bool> ABState::bestAB() const {
  if(aborted)
    return std::make_tuple(0,0,0);
  if(!parent)
    return std::make_tuple(a,b,aborted);

  std::tuple<int,int,bool> parentAB = this->parent->bestAB();
  return std::make_tuple(MAX(a,std::get<0>(parentAB)), MIN(b,std::get<1>(parentAB)), std::get<2>(parentAB));
}

void ABState::abort() {
  aborted = true;
}