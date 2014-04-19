
#include "Map.h"
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <climits>
#include <cstdio>
#include <iostream> 
#include "CycleTimer.h"
#include <boost/program_options.hpp>

using namespace std;

#define WIN 100000
#define LOSE -100000
#define DRAW -1
#define IN_PROGRESS 2

#define DEFAULT_DEPTH 7

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

namespace po = boost::program_options;
po::variables_map vm;

double vscoreTime;

int gameState(const Map& map) {
  if (map.MyX() == map.OpponentX() && map.MyY() == map.OpponentY()) return DRAW;
  bool my_status = map.IsWall(map.MyX(), map.MyY());
  bool opp_status = map.IsWall(map.OpponentX(), map.OpponentY());
  if (my_status && opp_status) return DRAW;
  if (!my_status && opp_status) return WIN;
  if (my_status && !opp_status) return LOSE;
  return IN_PROGRESS; 
}

/* The varonia score of a map is the number of squares that I can reach before my opponent, 
 * less the number of squares that my opponent can reach before me. */
int varonoiScore(const Map& map) {
  set <pair<int, int> >::iterator it;
  double start_time = CycleTimer::currentSeconds();

  //check for endgame scenarios
  int state = gameState(map);
  if (state != IN_PROGRESS) return state;

  //otherwise return varonoi score 
  vector< vector<bool> > grid = map.GetWalls();
  int my_score = 0, opp_score = 0;
  int x, y;

  //my_set and opp_set are essentially the "frontiers" of two simultaneous BFSs
  set< pair<int, int> > my_set, my_set_new, opp_set, opp_set_new;
  my_set.insert(make_pair(map.MyX(), map.MyY()));
  opp_set.insert(make_pair(map.OpponentX(), map.OpponentY()));

  while (!my_set.empty() || !opp_set.empty()) {
    my_set_new.clear();
    opp_set_new.clear();

    // add neighboring squares to my_set_new
    for (it = my_set.begin(); it != my_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;

      int newX[4] = {x, x+1, x, x-1};
      int newY[4] = {y-1, y, y+1, y};

      for(int i=0; i<4; i++){
        if (!map.IsWall(newX[i], newY[i]) && !grid[newX[i]][newY[i]]) {
          my_set_new.insert(make_pair(newX[i], newY[i]));
          // grid[newX[i]][newY[i]] = true;
        }
      }
    }

    // add neighboring squares to opp_set_new
    for (it = opp_set.begin(); it != opp_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;

      int newX[4] = {x, x+1, x, x-1};
      int newY[4] = {y-1, y, y+1, y};

      for(int i=0; i<4; i++){
        if (!map.IsWall(newX[i], newY[i]) && !grid[newX[i]][newY[i]]) {
          opp_set_new.insert(make_pair(newX[i], newY[i]));
          // grid[newX[i]][newY[i]] = true;
        }
      }
    }

    for(it=my_set_new.begin(); it != my_set_new.end(); ++it) {
      grid[(*it).first][(*it).second] = true;
    }
    for(it=opp_set_new.begin(); it != opp_set_new.end(); ++it) {
      grid[(*it).first][(*it).second] = true;
    }
    // printf("My set size: %d, opp set size: %d\n", my_set_new.size(), opp_set_new.size());

    //remove squares that are in both of our sets--we are equidistant from these squares
    //see http://stackoverflow.com/questions/2874441/deleting-elements-from-stl-set-while-iterating
    for (it = my_set_new.begin(); it != my_set_new.end(); ) {
      set <pair<int, int> >::iterator it2 = opp_set_new.find(*it);
      if (it2 != opp_set_new.end()) {
        opp_set_new.erase(it2);
        my_set_new.erase(it++);
      } else {
        ++it;
      }
    }

    //update scores and sets
    my_set = my_set_new;
    opp_set = opp_set_new;
    my_score += my_set.size();
    opp_score += opp_set.size();
  }

  double end_time = CycleTimer::currentSeconds();
  vscoreTime += (end_time - start_time);
  return my_score - opp_score;
}

int varonoiBlockScore(const Map& map, int block_id, std::vector<bool> visited) {
  std::pair<int,int> p = map.cutVertex(block_id);
  // if(p.first == -1 && p.second == -1){
  //   // Starting point is a cut vertex, handle accordingly
  //   return 0;
  // }

  // Otherwise starting in a block
  int block_score = map.blockVaronoi(block_id);
  std::set<int> neighbor_blocks = map.neighborBlocks(block_id);
  std::set<int>::iterator it;

  int max_score = block_score;
  for(it = neighbor_blocks.begin(); it!=neighbor_blocks.end(); it++){
    int child_block = *it;
    int total_score;
    if(!visited[child_block]){
      visited[child_block] = true;
      int child_score = varonoiBlockScore(map, child_block, visited);
      visited[child_block] = false;
      // BATTLEFRONT CONDITION
      if(true) {
        // If battlefront, find shortest path and update max
        int distance = 0;
        total_score = distance + child_score;
      } else {
        // Otherwise add value and update max
        total_score = block_score + child_score;
      }
      max_score = total_score > max_score ? total_score : max_score; 
    }
  }

  return max_score;
}

int varonoiBlockScoreWrapper(const Map& map) {
  std::vector<bool> visited(map.numBlocks(),false);
  int block_id = map.getBlock(map.MyX(),map.MyY());
  return varonoiBlockScore(map, block_id, visited);
}

pair<string, int> minimax (bool maxi, int depth, const Map &map) {
  int state = gameState(map);
  if (state != IN_PROGRESS) return make_pair("-",state);

  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  int player = maxi ? 1 : 0;
  int vscore_coeff = maxi ? 1 : -1;

  if(depth==0){
      return make_pair("",varonoiScore(map));
  } else {
    for(int i=0; i<4; i++){
      score[i] = vscore_coeff*minimax(!maxi, depth-1, Map(map, player, direction[i])).second;
    }
  }

  int best_score = INT_MIN;
  string best_dir = "";
  for(int i=0; i<4; i++){
    if(best_score < score[i]){
      best_score = score[i];
      best_dir = direction[i];
    }
  }

  if (maxi) return make_pair(best_dir, best_score);
  return make_pair(best_dir, -1*best_score);
}

pair<string, int> alphabeta (bool maxi, int depth, const Map &map, int a, int b) {
  int state = gameState(map);
  if (state != IN_PROGRESS) return make_pair("-",state);
  string direction[4] = {"NORTH", "SOUTH", "EAST", "WEST"};
  int score[4];
  string best_dir;
  int player = maxi ? 1 : 0;

  if(depth==0){
    return make_pair("",varonoiScore(map));
  }

  if(maxi){
    for(int i=0; i<4; i++){
      score[i] = alphabeta(!maxi, depth-1, Map(map, player, direction[i]), a, b).second;
      if(a < score[i]){
        a = score[i];
        best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    return make_pair(best_dir, a);
  } else {
    for(int i=0; i<4; i++){
      score[i] = alphabeta(!maxi, depth-1, Map(map, player, direction[i]), a, b).second;
      if(b > score[i]){
        b = score[i];
        best_dir = direction[i];
      }
      if (b<=a)
        break;
    }
    return make_pair(best_dir, b);
  }
}

string MakeMove(const Map& map) {
  int depth = vm.count("depth") ? vm["depth"].as<int>(): DEFAULT_DEPTH;
  if(vm.count("parallel")){
    // IMPLEMENT PARALLEL ALGORITHM HERE
    return "E"; 
  } else{
    if(vm.count("ab"))
      return alphabeta(true, depth, map, INT_MIN, INT_MAX).first; 
    else
      std::cout << minimax(true, depth, map).first << "ksdfd\n";
      return minimax(true, depth, map).first;
  }
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main(int argc, char* argv[]) {
  po::options_description desc("Options");   
  desc.add_options()
    ("help,h", "Print help messages")
    ("verbose,v", "Turn on verbose testing / benchmark")
    ("depth,d", po::value<int>(), "Maximum search depth")
    ("parallel,p", "Use parallel algorithm")
    ("ab", "alphabeta pruning");

  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << "Trobo options" << std::endl 
              << desc << std::endl; 
    return 0;
  }

  // If in verbose mode, print all sorts of things
  if (vm.count("verbose")){
    // std::cout << "Trobo testing mode:\n";
    while (true) {
      vscoreTime = 0.;

      // fprintf(stderr, "abc\n");
      Map map;
      double start_time = CycleTimer::currentSeconds();
      // fprintf(stderr, "def\n");
      Map::MakeMove(MakeMove(map));
      double end_time = CycleTimer::currentSeconds();
      fprintf(stderr, "Varonoi score on the starter map: %d\n", varonoiScore(map));
      fprintf(stderr, "Move took %.4f seconds\n", end_time - start_time);
      fprintf(stderr, "Spent %.4f seconds in varonoi function\n", vscoreTime);
    }
  } else {
    while (true) {
      // Otherwise we are on trobo competition mode
      Map map;
      Map::MakeMove(MakeMove(map));
    }
  } 
  return 0;
}
