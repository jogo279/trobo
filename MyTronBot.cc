
#include "Map.h"
#include "ABState.h"
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <cstdio>
#include <iostream> 
#include <boost/program_options.hpp>
#include <list>
#include <bitset>
#include <unordered_map>
#include <cilk/reducer_list.h>
#include <mutex>
#include <atomic>

using namespace std;

typedef pair<int,int> coord;
typedef cilk::reducer_list_append<std::pair<int, string>> reducer_list;

#define DEFAULT_TIME 1.0
#define START_DEPTH 6
#define STOP_DEPTH 32

#define step(dir,x,y) ((dir)=="NORTH"?(coord(x,y-1)) : ((dir)=="EAST"?(coord(x+1,y)):((dir)=="SOUTH"?(coord(x,y+1)):coord(x-1,y))))

typedef unsigned long long cache_key;
// Last 8 bits are the length, remaining bits are the key
#define CACHE_SIZE (8*sizeof(cache_key))
#define LEN_SIZE (8)
#define SEQ_SIZE (CACHE_SIZE-LEN_SIZE)
#define MOVE_LEN_MASK (0xFFULL << SEQ_SIZE)
#define MOVE_SEQ_MASK (~MOVE_LEN_MASK)

// Returns the length of the move sequence
#define MOVE_LEN(x) (0xFFULL & (x >> SEQ_SIZE))
// Returns the move sequence
#define MOVE_SEQ(x) (MOVE_SEQ_MASK & x)
// Increments the length of the move sequence by 1
#define INC_LEN(x) (x + (1ULL << SEQ_SIZE))
// Sets the length of the move sequence to l
#define SET_LEN(x,l) (MOVE_SEQ(x) | (l << SEQ_SIZE))
// Sets the move sequence to s
#define SET_SEQ(x,s) ((MOVE_LEN_MASK & x) | (s & MOVE_SEQ_MASK))

namespace po = boost::program_options;
po::variables_map vm;

double vscoreTime;

double start_time, timeLimit;
int cache_count;
int counter;
atomic<int> parallel_counter;
unordered_map<cache_key, char> cache;

int updateMoveSeq(cache_key move_seq, int move) {
  // return move_seq + (cache_key)(move << 2*depth);
  return (move_seq << 2)+move;
}

/* Given a history of moves, and a current move (at a current depth), cache
 * the move and return the new move_seq */
void cacheMove(cache_key move_seq, string move_str) {
  cache_count++;
  char move;
  // cache_key new_move_seq;
  int c = (int)move_str[0];
  // //fprintf(stderr, "size: %d, count: %d, key: %d, move %s\n", cache.size(),cache_count, move_seq, move_str.c_str());
  switch (c) {
    case 'n':
    case 'N':
      move = 0;
      break;
    case 's':
    case 'S':
      move = 1;
      break;
    case 'e':
    case 'E':
      move = 2;
      break;
    case 'w':
    case 'W':
      move = 3;
      break;
    default:
      return;
      break;
  }
  cache[move_seq] = move;
}

double timeLeft() {
  // //fprintf(stderr, "time left: %f\n", timeLimit - (CycleTimer::currentSeconds() - startTime));
  return timeLimit - (CycleTimer::currentSeconds() - start_time);
}

/* Once we've entered the endgame, we can ignore our opponent */
pair<string, int> endgame (int cur_depth, int max_depth, const Map &map, cache_key move_seq) {
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-", LOSE);

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  pair<string, int> child_eg;

  if (cur_depth==max_depth) {
    return make_pair("",map.Score()); 
  } else {
    for(int i=0; i<4; i++){
      coord next = step(direction[i],map.MyX(),map.MyY());
      if(map.IsEmpty(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_eg = endgame(cur_depth+1, max_depth, Map(map, 1, direction[i], leaf), updateMoveSeq(move_seq, i));
        child_eg.second++;
        if (child_eg.first == "T") return child_eg;
        score[i] = child_eg.second;
      }
      else {
        score[i] = LOSE;
      }
    }    
  }

  int best_score = INT_MIN;
  string best_dir = "";
  for(int i=0; i<4; i++){
    if(best_score < score[i]) {
      best_score = score[i];
      best_dir = direction[i];
    }
  }

  // if (vm.count("ab")) cacheMove(move_seq, best_dir);
  return make_pair(best_dir, best_score);
}

pair<string, int> parallel_endgame (int cur_depth, int max_depth, const Map &map, cache_key move_seq) {
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-", LOSE);

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};

  cilk::reducer_max_index<int, int> best_move;
  cilk::reducer_opadd<int> timeout;

  if (cur_depth==max_depth) {
    return make_pair("",map.Score()); 
  } else {
    cilk_for(int i=0; i<4; i++){
      coord next = step(direction[i],map.MyX(),map.MyY());
      pair<string, int> child_eg;
      if(map.IsEmpty(next.first, next.second)) {
        child_eg = parallel_endgame(cur_depth+1, max_depth, Map(map, 1, direction[i], cur_depth==max_depth-1), updateMoveSeq(move_seq, i));
        child_eg.second++;
      } else {
        child_eg= make_pair("-", LOSE);
      }
      best_move.calc_max(i, child_eg.second);
      if (child_eg.first=="T") timeout += 1;
    }    
  }

  if (timeout.get_value() > 0 && !vm.count("depth")) return make_pair("T", LOSE);
  //if (vm.count("ab")) cacheMove(move_seq, best_dir, cur_depth);
  return make_pair(direction[best_move.get_index()], best_move.get_value());
}

pair<string, int> minimax (bool maxi, int cur_depth, int max_depth, const Map &map, cache_key move_seq) {
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  pair<string, int> child_mm;
  int player = maxi ? 1 : 0;
  int vscore_coeff = maxi ? 1 : -1;

  if(cur_depth==max_depth){
      return make_pair("",map.Score());
  } else {
    for(int i=0; i<4; i++){
      coord next = maxi ? step(direction[i],map.MyX(),map.MyY()) : step(direction[i],map.OpponentX(),map.OpponentY());
      if(map.IsEmpty(next.first, next.second) || (!maxi && !map.IsWall(next.first,next.second))) {
        bool leaf = cur_depth==max_depth-1;
        child_mm = minimax(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), updateMoveSeq(move_seq, i));
        if (child_mm.first == "T") return child_mm;
        score[i] = vscore_coeff*child_mm.second;
      }
      else {
        score[i] = LOSE;
      }
    }
  }

  int best_score = INT_MIN;
  string best_dir = "";
  for(int i=0; i<4; i++){
    if(best_score < score[i]) {
      best_score = score[i];
      best_dir = direction[i];
    }
  }

  // cacheMove(move_seq, best_dir);
  if (maxi) return make_pair(best_dir, best_score);
  return make_pair(best_dir, -1*best_score);
}


pair<string, int> parallel_minimax (bool maxi, int cur_depth, int max_depth, const Map &map, cache_key move_seq) {
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int player = maxi ? 1 : 0;
  int vscore_coeff = maxi ? 1 : -1;


  cilk::reducer_max_index<int, int> best_move;
  cilk::reducer_opadd<int> timeout;

  if(cur_depth==max_depth){
      return make_pair("",map.Score());
  } else {
    cilk_for(int i=0; i<4; i++){
      coord next = maxi ? step(direction[i],map.MyX(),map.MyY()) : step(direction[i],map.OpponentX(),map.OpponentY());
      pair<string, int> child_mm;
      if(map.IsEmpty(next.first, next.second) || (!maxi && !map.IsWall(next.first,next.second))) {
        child_mm = parallel_minimax(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], cur_depth==max_depth-1), updateMoveSeq(move_seq, i));
        child_mm.second *= vscore_coeff;
      } else {
        child_mm = make_pair("-", LOSE);
      }
      best_move.calc_max(i, child_mm.second);
      if (child_mm.first=="T") timeout += 1;
    }
  }

  if (timeout.get_value() > 0 && !vm.count("depth")) return make_pair("T", LOSE);
  return make_pair(direction[best_move.get_index()], vscore_coeff*best_move.get_value());
}


pair<string, int> alphabeta (bool maxi, int cur_depth, int max_depth, const Map &map, int a, int b, cache_key move_seq, reducer_list &write_buffer) {
  counter++;
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());


  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  pair<string, int> child_ab;
  string best_dir;
  int player = maxi ? 1 : 0;
  std::unordered_map<cache_key, char>::iterator it;
  std::list<int> ord_children;
  for (int i = 0; i < 4; i++) ord_children.push_back(i);
  int i, best_guess;

  if(cur_depth == max_depth){
    return make_pair("",map.Score());
  }

  it = cache.find(move_seq);
  if (it != cache.end()) {
    int best_guess = it->second;
    ord_children.remove(best_guess);
    ord_children.push_front(best_guess);
  }

  if(maxi){
    int local_max = INT_MIN;
    for(std::list<int>::iterator it = ord_children.begin(); it !=ord_children.end(); ++it){
      i = *it;
      coord next = step(direction[i],map.MyX(),map.MyY());
      if(map.IsEmpty(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_ab = alphabeta(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), a, b, updateMoveSeq(move_seq, i), write_buffer);
        if (child_ab.first == "T") return child_ab;
        score[i] = child_ab.second;
      } else {
        score[i] = LOSE;
      }
      if(local_max < score[i]){
        local_max = score[i];
        best_dir = direction[i];
      }
      if(a < score[i]){
        a = score[i];
        // best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    cacheMove(move_seq, best_dir);
    write_buffer.push_back(std::pair<int,string>(move_seq, best_dir));
    return make_pair(best_dir, a);
  } else {
    int local_min=INT_MAX;
    for(std::list<int>::iterator it = ord_children.begin(); it !=ord_children.end(); ++it){
      i = *it;
      coord next = step(direction[i],map.OpponentX(),map.OpponentY());
      if(!map.IsWall(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_ab = alphabeta(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), a, b, updateMoveSeq(move_seq, i), write_buffer);
        if (child_ab.first == "T") return child_ab;
        score[i] = child_ab.second;
      } else {
        score[i] = WIN;
      }
      if(local_min > score[i]){
        local_min = score[i];
        best_dir = direction[i];
      }
      if(b > score[i]){
        b = score[i];
        // best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    cacheMove(move_seq, best_dir);
    write_buffer.push_back(std::pair<int,string>(move_seq, best_dir));
    return make_pair(best_dir, b);
  }
}

pair<string, int> parallel_alphabeta (bool maxi, int cur_depth, int max_depth, const Map &map, int a, int b, cache_key move_seq, reducer_list &write_buffer) {
  //fprintf(stderr,"depth: %d, a: %d, b: %d, counter: %d\n", cur_depth, a, b, parallel_counter.load());
  parallel_counter ++;

  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());
  if(cur_depth == max_depth) return make_pair("",map.Score());

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int player = maxi ? 1 : 0;
  int best_guess = -1;
  std::vector<int> ord_children;

  std::unordered_map<cache_key, char>::iterator it = cache.find(move_seq);
  if (it != cache.end()) {
    coord next = maxi ? step(direction[it->second],map.MyX(),map.MyY()) : step(direction[it->second], map.OpponentX(), map.OpponentY());
    if (map.IsEmpty(next.first, next.second)) {
      best_guess = it->second;
      ord_children.push_back(best_guess);
    }
  }

  for (int i = 0; i < 4; i++) {
    // //fprintf(stderr, "i: %d, best guess: %d\n",i,best_guess);
    coord next = maxi ? step(direction[i],map.MyX(),map.MyY()) : step(direction[i], map.OpponentX(), map.OpponentY());
    if((map.IsEmpty(next.first, next.second) || (!maxi && !map.IsWall(next.first,next.second))) && best_guess != i) ord_children.push_back(i);
  }

  if (ord_children.size() == 0) return make_pair("L", maxi ? LOSE : WIN);

  pair<string, int> child_ab = parallel_alphabeta(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[0]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[0]), write_buffer);

  if(maxi){
    cilk::reducer_max_index<int, int> best_move;
    cilk::reducer_opadd<int> timeout;
    best_move.calc_max(ord_children[0], child_ab.second);
    
    if (child_ab.first == "T") return child_ab;
    if(a < child_ab.second) a = child_ab.second;
    
    if (b>a && ord_children.size()>1) {
      cilk_for(int i = 1; i < ord_children.size(); i++) {
        pair<string, int> child_ab_parr = parallel_alphabeta(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
        if (child_ab_parr.first == "T") timeout += 1;
        best_move.calc_max(ord_children[i], child_ab_parr.second);
      }
    }

    if (timeout.get_value() > 0) return make_pair("T", LOSE);
    write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
    // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
    return make_pair(direction[best_move.get_index()], best_move.get_value());

  } else {
    cilk::reducer_min_index<int, int> best_move;
    cilk::reducer_opadd<int> timeout;
    best_move.calc_min(ord_children[0], child_ab.second);
    if (child_ab.first == "T") return child_ab;
    if(b > child_ab.second) b = child_ab.second;
    
    if (b>a && ord_children.size() > 1) {
      cilk_for(int i = 1; i < ord_children.size(); i++) {
        pair<string, int> child_ab_parr = parallel_alphabeta(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
        if (child_ab_parr.first == "T") timeout += 1;
        best_move.calc_min(ord_children[i], child_ab_parr.second);
      }
    }

    if (timeout.get_value() > 0) return make_pair("T", LOSE);
    write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
    // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
    return make_pair(direction[best_move.get_index()], best_move.get_value());
  }
}

pair<string, int> parallel_alphabeta_abort (ABState * parent, bool maxi, int cur_depth, int max_depth, const Map &map, int a, int b, cache_key move_seq, reducer_list &write_buffer) {
  //fprintf(stderr,"depth: %d, a: %d, b: %d, counter: %d\n", cur_depth, a, b, parallel_counter.load());
  ABState cur = ABState(parent);
  if (cur.isAborted()) {
    return make_pair("A", LOSE);
  } else {
    parallel_counter ++;
  }
  if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());
  if(cur_depth == max_depth) return make_pair("",map.Score());

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int player = maxi ? 1 : 0;
  int best_guess = -1;
  std::vector<int> ord_children;

  std::unordered_map<cache_key, char>::iterator it = cache.find(move_seq);
  if (it != cache.end()) {
    coord next = maxi ? step(direction[it->second],map.MyX(),map.MyY()) : step(direction[it->second], map.OpponentX(), map.OpponentY());
    if (map.IsEmpty(next.first, next.second)) {
      best_guess = it->second;
      ord_children.push_back(best_guess);
    }
  }

  for (int i = 0; i < 4; i++) {
    // //fprintf(stderr, "i: %d, best guess: %d\n",i,best_guess);
    coord next = maxi ? step(direction[i],map.MyX(),map.MyY()) : step(direction[i], map.OpponentX(), map.OpponentY());
    if((map.IsEmpty(next.first, next.second) || (!maxi && !map.IsWall(next.first,next.second))) && best_guess != i) ord_children.push_back(i);
  }

  if (ord_children.size() == 0) return make_pair("L", maxi ? LOSE : WIN);

  pair<string, int> child_ab = parallel_alphabeta_abort(&cur, !maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[0]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[0]), write_buffer);

  if (cur.isAborted()) return make_pair("A", maxi ? LOSE : WIN);

  if(maxi){
    cilk::reducer_max_index<int, int> best_move;
    cilk::reducer_opadd<int> timeout;
    best_move.calc_max(ord_children[0], child_ab.second);
    
    if (child_ab.first == "T") return child_ab;
    if(a < child_ab.second) a = child_ab.second;
    
    if (b>a && ord_children.size()>1) {
      cilk_for(int i = 1; i < ord_children.size(); i++) {
        if (!cur.isAborted()) {
          pair<string, int> child_ab_parr = parallel_alphabeta_abort(&cur, !maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
          if (child_ab_parr.first == "T") timeout += 1;
          best_move.calc_max(ord_children[i], child_ab_parr.second);
          if (best_move.get_value() >= b) cur.abort();
        }
      }
    }

    if (timeout.get_value() > 0) return make_pair("T", LOSE);
    write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
    // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
    return make_pair(direction[best_move.get_index()], best_move.get_value());

  } else {
    cilk::reducer_min_index<int, int> best_move;
    cilk::reducer_opadd<int> timeout;
    best_move.calc_min(ord_children[0], child_ab.second);
    if (child_ab.first == "T") return child_ab;
    if(b > child_ab.second) b = child_ab.second;
    
    if (b>a && ord_children.size() > 1) {
      cilk_for(int i = 1; i < ord_children.size(); i++) {
        if (!cur.isAborted()) {
          pair<string, int> child_ab_parr = parallel_alphabeta_abort(&cur, !maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), a, b, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
          if (child_ab_parr.first == "T") timeout += 1;
          best_move.calc_min(ord_children[i], child_ab_parr.second);
          if (best_move.get_value() <= a) cur.abort();
        }
      }
    }

    if (timeout.get_value() > 0) return make_pair("T", LOSE);
    write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
    // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
    return make_pair(direction[best_move.get_index()], best_move.get_value());
  }
}

// pair<string, int> parallel_alphabeta_abort (bool maxi, int cur_depth, int max_depth, const Map &map, ABState *prev, cache_key move_seq, reducer_list &write_buffer) {

//   parallel_counter++;
//   std::atomic<int> a(prev->getA());
//   std::atomic<int> b(prev->getB());
//   ABState cur = ABState(prev,&a,&b); 

//   if (timeLeft() < 0 && !vm.count("depth")) return make_pair("T", LOSE);
//   if (map.State() != IN_PROGRESS) return make_pair("-",map.State());
  

//   if (cur.isAborted())  { 
//     parallel_counter--;
//     return make_pair("A", LOSE);
//   }

//   if(cur_depth == max_depth) return make_pair("",map.Score());

//   // int ancesterA = cur.bestA();
//   // int ancesterB = cur.bestB();
//   // if(ancesterA > a.load()) cur.setA(ancesterA);
//   // if(ancesterB < b.load()) cur.setB(ancesterB);

//   string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
//   int player = maxi ? 1 : 0;
//   int best_guess = -1;
//   std::vector<int> ord_children;

//   std::unordered_map<cache_key, char>::iterator it = cache.find(move_seq);
//   if (it != cache.end()) {
//     coord next = maxi ? step(direction[it->second],map.MyX(),map.MyY()) : step(direction[it->second], map.OpponentX(), map.OpponentY());
//     if (map.IsEmpty(next.first, next.second)) {
//       best_guess = it->second;
//       ord_children.push_back(best_guess);
//     }
//   }

//   for (int i = 0; i < 4; i++) {
//     // //fprintf(stderr, "i: %d, best guess: %d\n",i,best_guess);
//     coord next = maxi ? step(direction[i],map.MyX(),map.MyY()) : step(direction[i], map.OpponentX(), map.OpponentY());
//     if((map.IsEmpty(next.first, next.second) || (!maxi && !map.IsWall(next.first,next.second))) && best_guess != i) ord_children.push_back(i);
//   }

//   if (ord_children.size() == 0) return make_pair("L", maxi ? LOSE : WIN);

//   pair<string, int> child_ab = parallel_alphabeta_abort(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[0]], cur_depth==max_depth-1), &cur, updateMoveSeq(move_seq, ord_children[0]), write_buffer);
  
//   if (cur.isAborted()) return make_pair("A", maxi ? LOSE : WIN);
//   // ancesterA = cur.bestA();
//   // ancesterB = cur.bestB();
//   // if(ancesterA > a.load()) cur.setA(ancesterA);
//   // if(ancesterB < b.load()) cur.setB(ancesterB);

//   if(maxi){
//     cilk::reducer_max_index<int, int> best_move;
//     cilk::reducer_opadd<int> timeout;
//     best_move.calc_max(ord_children[0], child_ab.second);
    
//     if (child_ab.first == "T") return child_ab;
//     if(a.load() < child_ab.second) a.store(child_ab.second);
    
//     if (b>a && ord_children.size()>1) {
//       cilk_for(int i = 1; i < ord_children.size(); i++) {
//         if(!cur.isSelfAborted()){
//           pair<string, int> child_ab_parr = parallel_alphabeta_abort(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), &cur, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
//           if (child_ab_parr.first == "T") timeout += 1;
//           if (child_ab_parr.second > a.load()) cur.setA(child_ab_parr.second);

//           best_move.calc_max(ord_children[i], child_ab_parr.second);
//           if(child_ab_parr.first == "A" || child_ab_parr.second >= b.load()) cur.abort();
//         }
//       }
//     }

//     if (timeout.get_value() > 0) return make_pair("T", LOSE);
//     write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
//     // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
//     return make_pair(direction[best_move.get_index()], best_move.get_value());

//   } else {
//     cilk::reducer_min_index<int, int> best_move;
//     cilk::reducer_opadd<int> timeout;
//     best_move.calc_min(ord_children[0], child_ab.second);
//     if (child_ab.first == "T") return child_ab;
//     if(b.load() > child_ab.second) b.store(child_ab.second);
    
//     if (b>a && ord_children.size() > 1) {
//       cilk_for(int i = 1; i < ord_children.size(); i++) {
//         if(!cur.isSelfAborted()){
//           pair<string, int> child_ab_parr = parallel_alphabeta_abort(!maxi, cur_depth + 1, max_depth, Map(map, player, direction[ord_children[i]], cur_depth==max_depth-1), &cur, updateMoveSeq(move_seq, ord_children[i]), write_buffer);
//           if (child_ab_parr.first == "T") timeout += 1;
//           if (child_ab_parr.second < b.load()) cur.setB(child_ab_parr.second);

//           best_move.calc_min(ord_children[i], child_ab_parr.second);

//           if(child_ab_parr.first == "A" || child_ab_parr.second <= a.load()) cur.abort();
//         }
//       }
//     }

//     if (timeout.get_value() > 0) return make_pair("T", LOSE);
//     write_buffer.push_back(std::pair<int,string>(move_seq, direction[best_move.get_index()]));
//     // cacheMove(move_seq, direction[best_move.get_index()], cur_depth);
//     return make_pair(direction[best_move.get_index()], best_move.get_value());
//   }
// }

pair<string, int> hybrid_start (int depth, const Map &map, ABState *init, reducer_list &write_buffer) {
  // int local[3][3];
  // //fprintf(stderr, "Starting hybrd\n");
  int numWalls=0;
  int numSteps = (depth+1)/2;
  int size=0;
  for(int i=0; i<=numSteps; i++){
    for(int j=0; j<=numSteps-i; j++){
      if(i==0 && j==0) continue;
      int x = map.MyX()+i;
      int y = map.MyY()+j;
      if(map.IsInBounds(x,y)){
        // //fprintf(stdout,"%d, %d\n",x,y);
        // local[i][j]=1;
        size++;
        if(map.IsWall(x,y) )
          numWalls++;
      }
      y = map.MyY()-j;
      if(map.IsInBounds(x,y)){
        // //fprintf(stdout,"%d, %d\n",x,y);
        // local[i][j]=1;
        size++;
        if(map.IsWall(x,y) )
          numWalls++;
      }
      x=map.MyX()-i;
      if(map.IsInBounds(x,y)){
        // //fprintf(stdout,"%d, %d\n",x,y);
        // local[i][j]=1;
        size++;
        if(map.IsWall(x,y) )
          numWalls++;
      }
      y = map.MyY()+j;
      if(map.IsInBounds(x,y)){
        // //fprintf(stdout,"%d, %d\n",x,y);
        // local[i][j]=1;
        size++;
        if(map.IsWall(x,y) )
          numWalls++;
      }
    }
  }

  double frac = ((double)numWalls/size);
  // frac /= size;
  //fprintf(stdout, "Number of walls %d to size %d, fraction: %f\n", numWalls, size, frac);

  int numAdjWalls=0;
  if(map.IsWall(map.MyX()+1,map.MyY()))
    numAdjWalls++;
  if(map.IsWall(map.MyX()-1,map.MyY()))
    numAdjWalls++;
  if(map.IsWall(map.MyX(),map.MyY()+1))
    numAdjWalls++;
  if(map.IsWall(map.MyX(),map.MyY()-1))
    numAdjWalls++;
  if(frac >= (0.0003911*depth*depth - 0.0195*depth + 0.4691) || depth <= 4)
    return alphabeta(true, 0, depth, map, INT_MIN, INT_MAX, 1, write_buffer);
  // else if(numWalls <= 2 && numAdjWalls <= 1)
    // return parallel_alphabeta_abort(true,0,depth, map, init, 1, write_buffer);
  else
    return parallel_alphabeta(true,0,depth, map, INT_MIN, INT_MAX, 1, write_buffer);
}


string MakeMove(const Map& map) {
  // int i=static_cast<double>(vm["time"].as<int>());
  start_time = CycleTimer::currentSeconds();
  timeLimit =(vm.count("time") ? double(vm["time"].as<int>()): DEFAULT_TIME) * .99;//multiply by .99 to leave error margin
  // //fprintf(stderr, "Timelimit: %d, %f, %f, %f\n",  i,double(i), (double)i, static_cast<double>(i));

  int depth = START_DEPTH;
  string cur_move, temp;

  // If only benching a particular depth
  if(vm.count("depth")) {
    depth = vm["depth"].as<int>();

    reducer_list write_buffer;
    parallel_alphabeta_abort(NULL, true,0,depth-1, map, INT_MIN, INT_MAX, 1, write_buffer);

    if (map.endGame()) { 
      if (vm.count("parallel")) {
        temp = parallel_endgame(0, depth, map, 1).first;
      } else {
        temp = endgame(0, depth, map, 1).first;
      }
    } else if (vm.count("ab")) { 
      
      start_time = CycleTimer::currentSeconds();
      counter=0;
      parallel_counter = 0;
      if(vm.count("parallel")) {
        if (vm.count("abort")) {
          temp = parallel_alphabeta_abort(NULL, true,0,depth, map, INT_MIN, INT_MAX, 1, write_buffer).first;
        } else {
          temp = parallel_alphabeta(true,0,depth, map, INT_MIN, INT_MAX,1, write_buffer).first;
        }
      } else {
        temp = alphabeta(true, 0, depth, map, INT_MIN, INT_MAX, 1, write_buffer).first;
      }
      const std::list<std::pair<int,string>> &write_buffer_list = write_buffer.get_value();
      for(std::list<std::pair<int,string>>::const_iterator i=write_buffer_list.begin(); i!= write_buffer_list.end(); i++){
        cacheMove((*i).first,((*i).second));
      } 
      
    } else if(vm.count("hybrid")){
        start_time = CycleTimer::currentSeconds();
        temp = hybrid_start(depth, map, NULL, write_buffer).first;
        const std::list<std::pair<int,string>> &write_buffer_list = write_buffer.get_value();
        for(std::list<std::pair<int,string>>::const_iterator i=write_buffer_list.begin(); i!= write_buffer_list.end(); i++){
          cacheMove((*i).first,((*i).second));
        } 
    } else {
      if(vm.count("parallel")) {
        temp = parallel_minimax(true, 0, depth, map, 1).first;
      } else {
        temp = minimax(true, 0, depth, map, 1).first;
      }
    }
    // //fprintf(stdout, "lkasdfdskl %s\n", temp.c_str());
    return temp;
  }

  while (timeLeft() > 0 && depth < STOP_DEPTH) {
    if (map.endGame()) {
      if (vm.count("parallel")) {
        temp = parallel_endgame(0, depth, map, 1).first;
      } else {
        temp = endgame(0, depth, map, 1).first;
      }
    } else if (vm.count("ab")) {
      reducer_list write_buffer;
      if(vm.count("parallel")) {
        if (vm.count("abort")) {
          temp = parallel_alphabeta_abort(NULL, true,0,depth, map, INT_MIN, INT_MAX, 1, write_buffer).first;
        } else {
          temp = parallel_alphabeta(true,0,depth, map, INT_MIN, INT_MAX,1, write_buffer).first;
        }
      } else {
        temp = alphabeta(true, 0, depth, map, INT_MIN, INT_MAX, 1, write_buffer).first;
      }
      const std::list<std::pair<int,string>> &write_buffer_list = write_buffer.get_value();
      for(std::list<std::pair<int,string>>::const_iterator i=write_buffer_list.begin(); i!= write_buffer_list.end(); i++){
        cacheMove((*i).first,((*i).second));
      }
    // } else if(vm.count("hybrid")){
    //   reducer_list write_buffer;
    //   temp = hybrid_start (depth, map, &init, write_buffer).first;
    //   const std::list<std::pair<int,string>> &write_buffer_list = write_buffer.get_value();
    //   for(std::list<std::pair<int,string>>::const_iterator i=write_buffer_list.begin(); i!= write_buffer_list.end(); i++){
    //     cacheMove((*i).first,((*i).second));
    //   } 
    } else {
      if(vm.count("parallel")) {
        temp = parallel_minimax(true, 0, depth, map, 1).first;
      } else {
        temp = minimax(true, 0, depth, map, 1).first;
      }
    }
    if (temp != "T") cur_move = temp;
    //fprintf(stderr, "Depth: %d, Move: %s, Time Left: %.4f\n", depth, temp.c_str(), timeLeft());
    depth ++;
  }
  return cur_move;
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main(int argc, char* argv[]) {

  po::options_description desc("Options");   
  desc.add_options()
    ("help,h", "Print help messages")
    ("verbose,v", "Turn on verbose testing / benchmark")
    ("time,t", po::value<int>(), "Time limit (ms)")
    ("parallel,p", "Use parallel algorithm")
    ("ab", "alphabeta pruning")
    ("abort", "abort")
    ("minimax", "standard minimax")
    ("hybrid,h", "hybrid algorithm")
    ("numworkers,n", po::value<string>(), "Number of CILK workers")
    ("test", "debugging line")
    ("depth,d", po::value<int>(), "Search to a particular depth d");

  po::store(po::parse_command_line(argc, argv, desc), vm);
  counter=0;

  if (vm.count("numworkers") && 0!= __cilkrts_set_param("nworkers",vm["numworkers"].as<string>().c_str())) {
    //fprintf(stderr, "Failed to set worker count\n");
    return 1;
  } else if (vm.count("numworkers")) {
    //fprintf(stderr, "Using %s workers\n", vm["numworkers"].as<string>().c_str());
  }

  if (vm.count("test")) {
    //fprintf(stderr, "Cache size: %d, length size: %d, seq size: %d\n", CACHE_SIZE, LEN_SIZE, SEQ_SIZE);
    std::cout << bitset<CACHE_SIZE>(MOVE_LEN_MASK) << '\n' << bitset<CACHE_SIZE>(MOVE_SEQ_MASK) << '\n';
    cache_key a = 0ULL;
    std::cout << bitset<CACHE_SIZE>(INC_LEN(a)) << '\n';
    std::cout << bitset<CACHE_SIZE>(SET_SEQ(a,0xF0F0ULL)) << '\n';
    std::cout << bitset<CACHE_SIZE>(SET_LEN(a,0xF0ULL)) << '\n';
    cache_key b = SET_LEN(SET_SEQ(a,0xF0F0ULL),0xF0ULL);
    std::cout << bitset<CACHE_SIZE>(b) << '\n';
    std::cout << bitset<CACHE_SIZE>(MOVE_LEN(b)) << '\n';
    std::cout << bitset<CACHE_SIZE>(MOVE_SEQ(b)) << '\n';
    return 0;
  }

  if (vm.count("help")) {
    std::cout << "Trobo options" << std::endl 
              << desc << std::endl; 
    return 0;
  }
      cache_count=0;

  // If in verbose mode, print all sorts of things
  if (vm.count("verbose")){
  // if (true){
    // std::cout << "Trobo testing mode:\n";
    while (true) {
      vscoreTime = 0.;
      Map map;
      
      start_time = CycleTimer::currentSeconds();
      Map::MakeMove(MakeMove(map));
      double end_time = CycleTimer::currentSeconds();
      if(vm.count("parallel"))
        fprintf(stderr, "%d\n", parallel_counter.load());
      else
        fprintf(stderr,"%d\n", counter);
    }
  } else {
    while (true) {
      // Otherwise we are on trobo competition mode
      Map map;
      Map::MakeMove(MakeMove(map));
      cache.clear();
    }
  } 
  return 0;
}
