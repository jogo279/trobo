// Map.cc

#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

Map::Map() {
  ReadFromFile(stdin);
  computeComponents();
  computeBlocks();
  computeVaronoi();
}

void Map::printStats() const {
  fprintf(stderr, "End Game:%d\n", endGame());
  fprintf(stderr, "Num Blocks:%d\n", numBlocks());
  fprintf(stderr, "\n\n");
}



void Map::computeVaronoi() {
  vector< vector<bool> > grid = GetWalls();
  int x, y;

  block_varonoi = vector<int>(num_blocks, 0);

  //my_set and opp_set are essentially the "frontiers" of two simultaneous BFSs
  set< pair<int, int> > my_set, my_set_new, opp_set, opp_set_new;
  if (!IsWall(player_one_x, player_one_y))
    my_set.insert(make_pair(player_one_x, player_one_y));
  if (!IsWall(player_two_x, player_two_y))
    my_set.insert(make_pair(player_two_x, player_two_y));

  while (!my_set.empty() || !opp_set.empty()) {
    my_set_new.clear();
    opp_set_new.clear();
    
    // add neighboring squares to my_set_new
    for (set <pair<int, int> >::iterator it = my_set.begin(); it != my_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;
      if (!IsWall(x, y-1) && !grid[x][y-1]) {
        my_set_new.insert(make_pair(x, y-1));
        grid[x][y-1] = true;
        block_varonoi[getBlock(x,y-1)] ++;
      }
      if (!IsWall(x+1, y) && !grid[x+1][y]) {
        my_set_new.insert(make_pair(x+1, y));
        grid[x+1][y] = true;
        block_varonoi[getBlock(x+1,y)] ++;
      }
      if (!IsWall(x, y+1) && !grid[x][y+1]) {
        my_set_new.insert(make_pair(x, y+1));
        grid[x][y+1] = true;
        block_varonoi[getBlock(x,y+1)] ++;
      }
      if (!IsWall(x-1, y) && !grid[x-1][y]) {
        my_set_new.insert(make_pair(x-1, y));
        grid[x-1][y] = true;
        block_varonoi[getBlock(x-1,y)] ++;
      }
    }

    // add neighboring squares to opp_set_new
    for (set <pair<int, int> >::iterator it = opp_set.begin(); it != opp_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;
      if (!IsWall(x, y-1) && !grid[x][y-1]) {
        opp_set_new.insert(make_pair(x, y-1));
        grid[x][y-1] = true;
        block_varonoi[getBlock(x,y-1)] --;
      }
      if (!IsWall(x+1, y) && !grid[x+1][y]) {
        opp_set_new.insert(make_pair(x+1, y));
        grid[x+1][y] = true;
        block_varonoi[getBlock(x+1,y)] --;
      }
      if (!IsWall(x, y+1) && !grid[x][y+1]) {
        opp_set_new.insert(make_pair(x, y+1));
        grid[x][y+1] = true;
        block_varonoi[getBlock(x,y+1)] --;
      }
      if (!IsWall(x-1, y) && !grid[x-1][y]) {
        opp_set_new.insert(make_pair(x-1, y));
        grid[x-1][y] = true;
        block_varonoi[getBlock(x-1,y)] --;
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
  }
}


void Map::findComponent(pair<int, int> point, int idx) {
  int x = point.first, y = point.second;

  set<pair<int,int> >::iterator it = points.find(make_pair(x, y-1));
  if (it != points.end()) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x, y+1));
  if (it != points.end()) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x-1, y));
  if (it != points.end()) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x+1, y));
  if (it != points.end()) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
}

void Map::computeComponents() {
  component_id =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));

  for (int i = 0; i < map_height; i++)
    for (int j = 0; j < map_width; j++)
      if (!IsWall(i,j))  points.insert(make_pair(i,j));

  num_components = 0;
  while(!points.empty()) {
    set<pair<int,int> >::iterator it = points.begin();
    component_id[(*it).first][(*it).second] = num_components;
    representative[num_components] = *it;
    findComponent(*it, num_components);
    points.erase(it);
    num_components++;
  }

  num_components++;
}


void Map::computeBlocks() {
  num_blocks = 0;

  num =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));
  low =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));

  pair<int, int> p = make_pair(-1,-1);
  for (int i = 0; i < map_width; i++)
    for (int j = 0; j < map_height; j++) 
      parent[i][j] = p;

  block_id =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));

  counter = 0;
  for (int i = 0; i < num_components; i++)
    assignNum(representative[i]);
  for (int i = 0; i < num_components; i++) {
    assignLow(representative[i]);
    //representative[i] is articulation point if has 2 children in DFS tree
    int children = 0, x = representative[i].first, y = representative[i].second;
    if (x !=0 && parent[x-1][y] == representative[i]) children ++;
    if (x !=map_width-1 && parent[x+1][y] == representative[i]) children ++;
    if (y !=0 && parent[x][y-1] == representative[i]) children ++;
    if (y !=map_height-1 && parent[x][y+1] == representative[i]) children ++;
    if (children > 1) {
      addCutVertex(x, y);
    }
  }

  // found the cut vertices--now fill in the remainder of the data structures
  for (int i = 0; i < num_components; i++) {
    int x = representative[i].first, y = representative[i].second;
    if (block_id[x][y] < 0) {
      blockDFS(x, y, num_blocks++);
      cut_location[num_blocks] = make_pair(-1,-1);
    }
  }


}

void Map::addCutVertex(int x, int y) {
  block_id[x][y] = num_blocks;
  block_size[num_blocks] = 1;
  cut_location[num_blocks] = make_pair(x, y);
  //neighbors, varonoi TODO
  num_blocks++;
}

void Map::blockDFS(int x, int y, int block_idx) {
  block_id[x][y] = block_idx;
  block_size[block_idx]++;
  blockDFSHelper(x,y+1,block_idx);
  blockDFSHelper(x,y-1,block_idx);
  blockDFSHelper(x+1,y,block_idx);
  blockDFSHelper(x-1,y,block_idx);
  //neighbors, varonoi TODO
}

 void Map::blockDFSHelper(int x, int y, int block_idx) {
  if (!IsWall(x,y) && block_id[x][y] < 0) {
    blockDFS(x,y,block_idx);
  } else if (!IsWall(x,y) && block_id[x][y] > 0 && block_id[x][y] != block_idx) {
    block_neighbors[block_idx].insert(block_id[x][y]);
    block_neighbors[block_id[x][y]].insert(block_idx);
  }
 }


void Map::assignNum(pair<int, int> v) {
  num[v.first][v.second] = ++ counter;
  if (v.second != map_height -1 && num[v.first][v.second + 1] <= 0) {
    parent[v.first][v.second + 1] = v;
    assignNum(make_pair(v.first, v.second + 1));
  }
  if (v.second != 0 && num[v.first][v.second - 1] <= 0) {
    parent[v.first][v.second - 1] = v;
    assignNum(make_pair(v.first, v.second - 1));
  }
  if (v.first != map_width -1 && num[v.first + 1][v.second ] <= 0) {
    parent[v.first + 1][v.second] = v;
    assignNum(make_pair(v.first + 1, v.second));
  }
  if (v.first != 0 && num[v.first - 1][v.second] <= 0) {
    parent[v.first - 1][v.second] = v;
    assignNum(make_pair(v.first - 1, v.second));
  }
}


void Map::assignLowHelper(int vfirst, int vsecond, int wfirst, int wsecond) {
  if (!IsWall(wfirst, wsecond)) {
    if (num[wfirst][wsecond] > num[vfirst][vsecond]) {
      assignLow(make_pair(wfirst, wsecond));
      if (low[wfirst][wsecond] >= num[vfirst][vsecond]) {
        addCutVertex(vfirst, vsecond);
      }
      low[vfirst][vsecond] = min(low[vfirst][vsecond], low[wfirst][wsecond]);
    } else {
      if (parent[vfirst][vsecond] != make_pair(wfirst, wsecond))
        low[vfirst][vsecond] = min(low[vfirst][vsecond], num[wfirst][wsecond]);
    }
  }  
}

void Map::assignLow(pair<int, int> v) {
  low[v.first][v.second] = num[v.first][v.second];
  assignLowHelper(v.first, v.second, v.first, v.second+1);
  assignLowHelper(v.first, v.second, v.first, v.second-1);
  assignLowHelper(v.first, v.second, v.first+1, v.second);
  assignLowHelper(v.first, v.second, v.first-1, v.second);
}





//TODO
bool Map::endGame() const {
  return component_id[player_one_x][player_one_y] != component_id[player_two_x][player_two_y];
}

Map::Map(const Map &other, int player, string direction) {

  player_one_x = other.MyX();
  player_one_y = other.MyY();
  player_two_x = other.OpponentX();
  player_two_y = other.OpponentY();
  map_width = other.Width();
  map_height = other.Height();
  is_wall = other.GetWalls();

  int c = (int)direction[0];
  switch (c) {
    case 'n':
    case 'N':
      if (player == 1) {
        is_wall[player_one_x][player_one_y] = true;
        player_one_y--;
      } else {
        is_wall[player_two_x][player_two_y] = true;
        player_two_y--;
      }
      break;
    case 'e':
    case 'E':
      if (player == 1) {
        is_wall[player_one_x][player_one_y] = true;
        player_one_x++;
      } else {
        is_wall[player_two_x][player_two_y] = true;
        player_two_x++;
      }
      break;
    case 's':
    case 'S':
      if (player == 1) {
        is_wall[player_one_x][player_one_y] = true;
        player_one_y++;
      } else {
        is_wall[player_two_x][player_two_y] = true;
        player_two_y++;
      }
      break;
    default:
      if (player == 1) {
        is_wall[player_one_x][player_one_y] = true;
        player_one_x--;
      } else {
        is_wall[player_two_x][player_two_y] = true;
        player_two_x--;
      }
      break;
  }
  computeComponents();
  computeBlocks();
  computeVaronoi();
}

int Map::Width() const {
  return map_width;
}

int Map::Height()  const {
  return map_height;
}

bool Map::IsWall(int x, int y) const {
  if (x < 0 || y < 0 || x >= map_width || y >= map_height) {
    return true;
  } else {
    return is_wall[x][y];
  }
}

vector< vector<bool> > Map::GetWalls() const {
  return is_wall;
}

int Map::MyX() const {
  return player_one_x;
}

int Map::MyY() const {
  return player_one_y;
}

int Map::OpponentX() const {
  return player_two_x;
}

int Map::OpponentY() const {
  return player_two_y;
}

void Map::MakeMove(const std::string& move) {
  if (move.length() == 0) {
    fprintf(stderr, "ERROR: zero-length string passed to MakeMove(string)\n");
    MakeMove(0);
  } else {
    int c = (int)move[0];
    switch (c) {
    case 'n':
    case 'N':
      MakeMove(1);
      break;
    case 'e':
    case 'E':
      MakeMove(2);
      break;
    case 's':
    case 'S':
      MakeMove(3);
      break;
    case 'w':
    case 'W':
      MakeMove(4);
      break;
    default:
      fprintf(stderr, "Invalid string passed to MakeMove(string): %s\n"
	      "Move string must start with N, E, S, or W!", move.c_str());
      MakeMove(0);
      break;
    }
  }
}

void Map::MakeMove(int move) {
  fprintf(stdout, "%d\n", move);
  fflush(stdout);
}

int Map::numBlocks() const {
  return num_blocks;
}

int Map::getBlock(int x, int y) const {
  return block_id[x][y];
}

int Map::blockSize(int block_id) const {
  return block_size[block_id];
}

int Map::blockVaronoi(int block_id) const {
  return block_varonoi[block_id];
}

pair<int, int> Map::cutVertex(int block_id) const {
  return cut_location[block_id];
}

set<int> Map::neighborBlocks(int block_id) const {
  return block_neighbors[block_id];
}






void Map::ReadFromFile(FILE *file_handle) {
  int x, y, c;
  int num_items = fscanf(file_handle, "%d %d\n", &map_width, &map_height);
  if (feof(file_handle) || num_items < 2) {
    exit(0); // End of stream means end of game. Just exit.
  }
  is_wall =
    vector<vector<bool> >(map_width,
				    vector<bool>(map_height, false));
  x = 0;
  y = 0;
  while (y < map_height && (c = fgetc(file_handle)) != EOF) {
    switch (c) {
    case '\r':
      break;
    case '\n':
      if (x != map_width) {
      	fprintf(stderr, "x != width in Board_ReadFromStream\n");
      	return;
      }
      ++y;
      x = 0;
      break;
    case '#':
      if (x >= map_width) {
      	fprintf(stderr, "x >= width in Board_ReadFromStream\n");
      	return;
      }
      is_wall[x][y] = true;
      ++x;
      break;
    case ' ':
      if (x >= map_width) {
      	fprintf(stderr, "x >= width in Board_ReadFromStream\n");
      	return;
      }
      is_wall[x][y] = false;
      ++x;
      break;
    case '1':
      if (x >= map_width) {
      	fprintf(stderr, "x >= width in Board_ReadFromStream\n");
      	return;
      }
      is_wall[x][y] = false;
      player_one_x = x;
      player_one_y = y;
      ++x;
      break;
    case '2':
      if (x >= map_width) {
      	fprintf(stderr, "x >= width in Board_ReadFromStream\n");
      	return;
      }
      is_wall[x][y] = false;
      player_two_x = x;
      player_two_y = y;
      ++x;
      break;
    default:
      fprintf(stderr, "unexpected character %d in Board_ReadFromStream", c);
      return;
    }
  }
}
