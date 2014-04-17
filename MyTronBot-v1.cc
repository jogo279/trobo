
#include "Map.h"
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <climits>

using namespace std;

#define WIN 100000
#define LOSE -100000
#define DRAW -1
#define IN_PROGRESS 2


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
    for (set <pair<int, int> >::iterator it = my_set.begin(); it != my_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;
      if (!map.IsWall(x, y-1) && !grid[x][y-1]) {
        my_set_new.insert(make_pair(x, y-1));
        grid[x][y-1] = true;
      }
      if (!map.IsWall(x+1, y) && !grid[x+1][y]) {
        my_set_new.insert(make_pair(x+1, y));
        grid[x+1][y] = true;
      }
      if (!map.IsWall(x, y+1) && !grid[x][y+1]) {
        my_set_new.insert(make_pair(x, y+1));
        grid[x][y+1] = true;
      }
      if (!map.IsWall(x-1, y) && !grid[x-1][y]) {
        my_set_new.insert(make_pair(x-1, y));
        grid[x-1][y] = true;
      }
    }

    // add neighboring squares to opp_set_new
    for (set <pair<int, int> >::iterator it = opp_set.begin(); it != opp_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;
      if (!map.IsWall(x, y-1) && !grid[x][y-1]) {
        opp_set_new.insert(make_pair(x, y-1));
        grid[x][y-1] = true;
      }
      if (!map.IsWall(x+1, y) && !grid[x+1][y]) {
        opp_set_new.insert(make_pair(x+1, y));
        grid[x+1][y] = true;
      }
      if (!map.IsWall(x, y+1) && !grid[x][y+1]) {
        opp_set_new.insert(make_pair(x, y+1));
        grid[x][y+1] = true;
      }
      if (!map.IsWall(x-1, y) && !grid[x-1][y]) {
        opp_set_new.insert(make_pair(x-1, y));
        grid[x-1][y] = true;
      }
    }

    //remove squares that are in both of our sets--we are equidistant from these squares
    //see http://stackoverflow.com/questions/2874441/deleting-elements-from-stl-set-while-iterating
    for (set <pair<int, int> >::iterator it = my_set_new.begin(); it != my_set_new.end(); ) {
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
  return my_score - opp_score;
}


pair<string, int> minimax (bool maxi, int depth, const Map &map) {
  int state = gameState(map);
  if (state != IN_PROGRESS) return make_pair("-",state);

  int north_score, south_score, east_score, west_score;
  if (maxi) {
    if (depth==1) {
      north_score = varonoiScore(Map(map, 1, "NORTH"));
      south_score = varonoiScore(Map(map, 1, "SOUTH"));
      east_score = varonoiScore(Map(map, 1, "EAST"));
      west_score = varonoiScore(Map(map, 1, "WEST"));
    } else {
      north_score = minimax(false, depth-1, Map(map, 1, "NORTH")).second;
      south_score = minimax(false, depth-1, Map(map, 1, "SOUTH")).second;
      east_score = minimax(false, depth-1, Map(map, 1, "EAST")).second;
      west_score = minimax(false, depth-1, Map(map, 1, "WEST")).second;
    }
  } else {
    if (depth==1) {
      north_score = -1*varonoiScore(Map(map, 2, "NORTH"));
      south_score = -1*varonoiScore(Map(map, 2, "SOUTH"));
      east_score = -1*varonoiScore(Map(map, 2, "EAST"));
      west_score = -1*varonoiScore(Map(map, 2, "WEST"));
    } else {
      north_score = -1*minimax(true, depth-1, Map(map, 2, "NORTH")).second;
      south_score = -1*minimax(true, depth-1, Map(map, 2, "SOUTH")).second;
      east_score = -1*minimax(true, depth-1, Map(map, 2, "EAST")).second;
      west_score = -1*minimax(true, depth-1, Map(map, 2, "WEST")).second;
    }
  }
  int best_score = north_score;
  string best_dir = "NORTH";
  if (south_score > best_score) {
    best_score = south_score;
    best_dir = "SOUTH";
  }
  if (east_score > best_score) {
    best_score = east_score;
    best_dir = "EAST";
  }
  if (west_score > best_score) {
    best_score = west_score;
    best_dir = "WEST";
  }
  if (maxi) return make_pair(best_dir, best_score);
  return make_pair(best_dir, -1*best_score);
}

string MakeMove(const Map& map) {
  return minimax(true, 3, map).first;
}

// Ignore this function. It is just handling boring stuff for you, like
// communicating with the Tron tournament engine.
int main() {
  while (true) {
    Map map;
    Map::MakeMove(MakeMove(map));
  }
  return 0;
}
