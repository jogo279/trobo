#include "ABState.h"
#include <climits>
#include <cstddef>
#include <iostream> 

#define MAX(a,b) ((a)>(b))?(a):(b)
#define MIN(a,b) ((a)<(b))?(a):(b)

ABState::ABState(ABState * par, std::atomic<int> *A, std::atomic<int> *B) {
  count = 0;
  aborted = false;
  parent = par;
  a = A;
  b = B;
}

int ABState::getA() const {
  return a->load();
}

int ABState::getB() const {
  return b->load();
}
void ABState::setA(int val) {
  int prev_a = a->load();
  while(prev_a < val  && !a->compare_exchange_weak(prev_a,val));
  // a = val;
  // a->store(val);
}

void ABState::setB(int val) {
  int prev_b = b->load();
  while(prev_b > val && !b->compare_exchange_weak(prev_b, val));
  // b = val;
  // b->store(val);
}

int ABState::bestA() const {
  if(parent)
    return MAX(a->load(),parent->bestA());
  return a->load();
}

int ABState::bestB() const {
  if(parent)
    return MIN(b->load(),parent->bestB());
  return b->load();
}

int ABState::isAborted() const {
  return aborted || (parent && this->parent->isAborted());
}

// std::tuple<int,int,bool> ABState::bestAB() const {
//   if(aborted)
//     return std::make_tuple(0,0,0);
//   if(!parent)
//     return std::make_tuple(a,b,aborted);

//   std::tuple<int,int,bool> parentAB = this->parent->bestAB();
//   return std::make_tuple(MAX(a,std::get<0>(parentAB)), MIN(b,std::get<1>(parentAB)), std::get<2>(parentAB));
// }

void ABState::abort() {
  aborted = true;
}