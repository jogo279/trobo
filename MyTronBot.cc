
#include "Map.h"
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <cstdio>
#include <iostream> 
#include "CycleTimer.h"
#include <boost/program_options.hpp>
#include <unordered_map>
#include <list>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h> 
#include <cilk/reducer_max.h>
#include <cilk/reducer_opadd.h>

using namespace std;

typedef pair<int,int> coord;

#define DEFAULT_TIME 1.0
#define START_DEPTH 6

#define step(dir,x,y) ((dir)=="NORTH"?(coord(x,y-1)) : ((dir)=="EAST"?(coord(x+1,y)):((dir)=="SOUTH"?(coord(x,y+1)):coord(x-1,y))))

namespace po = boost::program_options;
po::variables_map vm;

double vscoreTime;

double startTime, timeLimit;
unordered_map<int, char> cache;

int updateMoveSeq(int move_seq, int move, int depth) {
  return move_seq + (move << 2*depth);
}

/* Given a history of moves, and a current move (at a current depth), cache
 * the move and return the new move_seq */
void cacheMove(int move_seq, string move_str, int depth) {
  char move;
  int new_move_seq;
  int c = (int)move_str[0];
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
  return timeLimit - (CycleTimer::currentSeconds() - startTime);
}

/* Once we've entered the endgame, we can ignore our opponent */
pair<string, int> endgame (int cur_depth, int max_depth, const Map &map, int move_seq) {
  if (timeLeft() < 0) return make_pair("T", LOSE);
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
        child_eg = endgame(cur_depth+1, max_depth, Map(map, 1, direction[i], leaf), updateMoveSeq(move_seq, i, cur_depth));
        if (child_eg.first == "T") return child_eg;
        score[i] = child_eg.second + 1;
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

  cacheMove(move_seq, best_dir, cur_depth);
  return make_pair(best_dir, best_score);
}

pair<string, int> minimax (bool maxi, int cur_depth, int max_depth, const Map &map, int move_seq) {
  if (timeLeft() < 0) return make_pair("T", LOSE);
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
      if(map.IsEmpty(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_mm = minimax(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), updateMoveSeq(move_seq, i, cur_depth));
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

  cacheMove(move_seq, best_dir, cur_depth);
  if (maxi) return make_pair(best_dir, best_score);
  return make_pair(best_dir, -1*best_score);
}


pair<string, int> parallel_minimax (bool maxi, int cur_depth, int max_depth, const Map &map, int move_seq) {
  if (timeLeft() < 0) return make_pair("T", LOSE);
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
      if(map.IsEmpty(next.first, next.second)) {
        child_mm = parallel_minimax(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], cur_depth==max_depth-1), updateMoveSeq(move_seq, i, cur_depth));
      } else {
        child_mm = make_pair("-", maxi ? LOSE : WIN);
      }
      best_move.calc_max(i, vscore_coeff*child_mm.second);
      if (child_mm.first=="T") timeout += 1;
    }
  }

  if (timeout.get_value() > 0) return make_pair("T", LOSE);
  return make_pair(direction[best_move.get_index()], best_move.get_value());
}


pair<string, int> alphabeta (bool maxi, int cur_depth, int max_depth, const Map &map, int a, int b, int move_seq) {
  if (timeLeft() < 0) return make_pair("T", LOSE);
  if (map.State() != IN_PROGRESS) return make_pair("-",map.State());
  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  pair<string, int> child_ab;
  string best_dir;
  int player = maxi ? 1 : 0;
  std::unordered_map<int, char>::iterator it;
  std::list<int> indices;
  for (int i = 0; i < 4; i++) indices.push_back(i);
  int i, best_guess;

  if(cur_depth == max_depth){
    return make_pair("",map.Score());
  }

  it = cache.find(move_seq);
  if (it != cache.end()) {
    int best_guess = it->second;
    indices.remove(best_guess);
    indices.push_front(best_guess);
  }

  if(maxi){
    for(std::list<int>::iterator it = indices.begin(); it !=indices.end(); ++it){
      i = *it;
      coord next = step(direction[i],map.MyX(),map.MyY());
      if(map.IsEmpty(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_ab = alphabeta(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), a, b, updateMoveSeq(move_seq, i, cur_depth));
        if (child_ab.first == "T") return child_ab;
        score[i] = child_ab.second;
      } else {
        score[i] = LOSE;
      }
      if(a < score[i]){
        a = score[i];
        best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    cacheMove(move_seq, best_dir, cur_depth);
    return make_pair(best_dir, a);
  } else {
    for(std::list<int>::iterator it = indices.begin(); it !=indices.end(); ++it){
      i = *it;
      coord next = step(direction[i],map.OpponentX(),map.OpponentY());
      if(map.IsEmpty(next.first, next.second)) {
        bool leaf = cur_depth==max_depth-1;
        child_ab = alphabeta(!maxi, cur_depth+1, max_depth, Map(map, player, direction[i], leaf), a, b, updateMoveSeq(move_seq, i, cur_depth));
        if (child_ab.first == "T") return child_ab;
        score[i] = child_ab.second;
      } else {
        score[i] = WIN;
      }
      if(b > score[i]){
        b = score[i];
        best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    cacheMove(move_seq, best_dir, cur_depth);
    return make_pair(best_dir, b);
  }
}

string MakeMove(const Map& map) {
  startTime = CycleTimer::currentSeconds();
  timeLimit =(vm.count("time") ? vm["time"].as<double>(): DEFAULT_TIME) * .99;//multiply by .99 to leave error margin

  int depth = START_DEPTH;
  string cur_move, temp;

  while (timeLeft() > 0) {
    if (map.endGame()) {
      if (vm.count("parallel")) {
        //TODO: parallel endgame
        temp = endgame(0, depth, map, 0).first;
      } else {
        temp = endgame(0, depth, map, 0).first;
      }
    } else if (vm.count("ab")) {
      temp = alphabeta(true, 0, depth, map, INT_MIN, INT_MAX, 0).first;
    } else {
      if(vm.count("parallel")) {
        temp = parallel_minimax(true, 0, depth, map, 0).first;
      } else {
        temp = minimax(true, 0, depth, map, 0).first;
      }
    }
    if (temp != "T") cur_move = temp;
    depth ++;
    fprintf(stderr, "Depth: %d, Move: %s, Time Left: %.4f\n", depth, temp.c_str(), timeLeft());
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
    ("minimax", "standard minimax")
    ("numworkers,n", po::value<string>(), "Number of CILK workers");

  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("numworkers") && 0!= __cilkrts_set_param("nworkers",vm["numworkers"].as<string>().c_str())) {
    printf("Failed to set worker count\n");
    return 1;
  } else if (vm.count("numworkers")) {
    printf("Using %s workers\n", vm["numworkers"].as<string>().c_str());
  }

  if (vm.count("help")) {
    std::cout << "Trobo options" << std::endl 
              << desc << std::endl; 
    return 0;
  }

  // If in verbose mode, print all sorts of things
  if (vm.count("verbose")){
  // if (true){
    // std::cout << "Trobo testing mode:\n";
    while (true) {
      vscoreTime = 0.;
      Map map;

      // fprintf(stderr, "\n\nStart of move: %d (should be %d)\n", map.IsWall(map.MyX(),map.MyY()), map.IsWall(0,0));
      // fprintf(stderr, "Varonoi score  recursive on the starter map: %d\n", map.Score());
      double start_time = CycleTimer::currentSeconds();
      // fprintf(stderr, "def\n");
      Map::MakeMove(MakeMove(map));
      double end_time = CycleTimer::currentSeconds();
      fprintf(stderr, "Move took %.4f seconds\n", end_time - start_time);
      // fprintf(stderr, "Spent %.4f seconds in varonoi function\n", vscoreTime);
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
