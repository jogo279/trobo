// Map.cc

#include "Map.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

Map::Map() {
  ReadFromFile(stdin);
  computeComponents();
}

int Map::Score() const {
  return score;
}

int Map::State() const {
  if (MyX() == OpponentX() && MyY() == OpponentY()) return DRAW;
  bool my_status = IsWall(MyX(), MyY());
  bool opp_status = IsWall(OpponentX(), OpponentY());
  bool collision_status = (MyX() == OpponentX() && MyY() == OpponentY());
  if ((my_status && opp_status) || collision_status) return DRAW;
  if (!my_status && opp_status) return WIN;
  if (my_status && !opp_status) return LOSE;
  return IN_PROGRESS; 
}

void Map::printStats() const {
  fprintf(stderr, "Stats!\n");
  fprintf(stderr, "Player 1 (%d, %d); Player 2 (%d, %d)\n", player_one_x, player_one_y, player_two_x, player_two_y);
  fprintf(stderr, "End Game:%d\n", endGame());
  fprintf(stderr, "Num Blocks:%d\n", numBlocks());
  for (int i = 0; i < numBlocks(); i++) {
    fprintf(stderr, "Varonoi for block %d is (my,opp)=(%d,%d)\n", i, blockVaronoi(i,1), blockVaronoi(i,2));
    fprintf(stderr, "Block %d battlefront status: %d\n", i, blockBattlefront(i));
  }
  fprintf(stderr, "\n\n");
}

void Map::printBlocks() const {
  for (int j = 0; j < map_height; j++){
    for (int i = 0; i < map_width; i++){
      int id = getBlock(i,j);
      if(id == -1)
        fprintf(stderr, "##");
      else if (i == player_one_x && j == player_one_y)
        fprintf(stderr, "**");
      else if (i == player_two_x && j == player_two_y)
        fprintf(stderr, "$$");
      else
        fprintf(stderr, "%02i", id);
    }
    fprintf(stderr, "\n");
  }
}



int Map::vertexScore(int i, int j) {
  // return 1;
  int edges = 0;
  if (!IsWall(i-1,j)) edges ++;
  if (!IsWall(i+1,j)) edges ++;
  if (!IsWall(i,j-1)) edges ++;
  if (!IsWall(i,j+1)) edges ++;
  return 55 + 194*edges;
}


void Map::computeVaronoi() {
  // fprintf(stderr, "starting computation of vblocks\n");
  grid = GetWalls();
  int x, y; 

  

  my_block_varonoi = vector<int>(num_blocks, 0);
  opp_block_varonoi = vector<int>(num_blocks, 0);

  //my_set and opp_set are essentially the "frontiers" of two simultaneous BFSs
  if (!IsWall(player_one_x, player_one_y)){
    my_set.insert(make_pair(player_one_x, player_one_y));
    grid[player_one_x][player_one_y] = true;
  }
  if (!IsWall(player_two_x, player_two_y)){
    opp_set.insert(make_pair(player_two_x, player_two_y));
    grid[player_two_x][player_two_y] = true;
  }


  while (!my_set.empty() || !opp_set.empty()) {
    my_set_new.clear();
    opp_set_new.clear();

    // add neighboring squares to my_set_new
    for (set <pair<int, int> >::iterator it = my_set.begin(); it != my_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;

      int newX[4] = {x, x+1, x, x-1};
      int newY[4] = {y-1, y, y+1, y};

      for(int i=0; i<4; i++){
        if (!IsWall(newX[i], newY[i]) && !grid[newX[i]][newY[i]]) {
          // fprintf(stderr, "Adding %d,%d to player 1 in block %d\n", newX[i], newY[i], getBlock(newX[i], newY[i]));
          my_set_new.insert(make_pair(newX[i], newY[i]));

          // block_varonoi[getBlock(newX[i], newY[i])] += vertexScore(newX[i], newY[i]);
          // if (block_varonoi[getBlock(newX[i], newY[i])] < 0) battlefront[getBlock(newX[i], newY[i])] = true;
        }
      }
    }

    // add neighboring squares to opp_set_new
    for (set <pair<int, int> >::iterator it = opp_set.begin(); it != opp_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;

      int newX[4] = {x, x+1, x, x-1};
      int newY[4] = {y-1, y, y+1, y};

      for(int i=0; i<4; i++){
        if (!IsWall(newX[i], newY[i]) && !grid[newX[i]][newY[i]]) {
          // fprintf(stderr, "Adding %d,%d to player 2 in block %d\n", newX[i], newY[i], getBlock(newX[i], newY[i]));
          opp_set_new.insert(make_pair(newX[i], newY[i]));

          // block_varonoi[getBlock(newX[i], newY[i])] -= vertexScore(newX[i], newY[i]);
          // if (block_varonoi[getBlock(newX[i], newY[i])] > 0) battlefront[getBlock(newX[i], newY[i])] = true;
        }
      }
    }

    //remove squares that are in both of our sets--we are equidistant from these squares
    //see http://stackoverflow.com/questions/2874441/deleting-elements-from-stl-set-while-iterating
    for (set <pair<int, int> >::iterator it = my_set_new.begin(); it != my_set_new.end(); ) {
      grid[(*it).first][(*it).second] = true;
      set <pair<int, int> >::iterator it2 = opp_set_new.find(*it);
      if (it2 != opp_set_new.end()) {
        opp_set_new.erase(it2);
        my_set_new.erase(it++);
      } else {
        // fprintf(stderr, "Adding %d,%d to player 1 in block %d\n", (*it).first, (*it).second, getBlock((*it).first, (*it).second));
        my_block_varonoi[getBlock((*it).first, (*it).second)] += vertexScore((*it).first, (*it).second);
        ++it;
      }
    }

    for (set <pair<int, int> >::iterator it = opp_set_new.begin(); it != opp_set_new.end(); it++) {
      // fprintf(stderr, "Adding %d,%d to player 2 in block %d\n", (*it).first, (*it).second, getBlock((*it).first, (*it).second));
      grid[(*it).first][(*it).second] = true;
      opp_block_varonoi[getBlock((*it).first, (*it).second)] += vertexScore((*it).first, (*it).second);
      if (my_block_varonoi[getBlock((*it).first, (*it).second)] > 0) battlefront[getBlock((*it).first, (*it).second)] = true;
    }

    //update scores and sets
    my_set = my_set_new;
    opp_set = opp_set_new;

  }

  // fprintf(stderr, "ending computation of vblocks\n");
}


void Map::findComponent(pair<int, int> point, int idx) {
  int x = point.first, y = point.second;

  set<pair<int,int> >::iterator it = points.find(make_pair(x, y-1));
  if (it != points.end() && component_id[x][y-1] == -1) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x, y+1));
  if (it != points.end() && component_id[x][y+1] == -1) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x-1, y));
  if (it != points.end() && component_id[x-1][y] == -1) {
    component_id[(*it).first][(*it).second] = idx;
    findComponent(*it, idx);
    points.erase(it);
  }
  it = points.find(make_pair(x+1, y));
  if (it != points.end() && component_id[x+1][y] == -1) {
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
      if (IsEmpty(i,j)) points.insert(make_pair(i,j));

  num_components = 0;
  while(!points.empty()) {
    set<pair<int,int> >::iterator it = points.begin();
    component_id[(*it).first][(*it).second] = num_components;
    representative.push_back(*it);
    findComponent(*it, num_components);
    points.erase(it);
    num_components++;
  }
}


void Map::computeBlocks() {
  num_blocks = 0;

  num =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));
  low =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));

  parent = 
    vector<vector<pair<int,int> > >(map_height);

  pair<int, int> p = make_pair(-1,-1);
  for (int i = 0; i < map_height; i++) {
    parent[i] = vector<pair<int, int> >(map_width);
    for (int j = 0; j < map_width; j++) 
      parent[i][j] = p;
  }

  block_id =
    vector<vector<int> >(map_width,
            vector<int>(map_height, -1));

  counter = 0;
  for (int i = 0; i < num_components; i++) {
    calculateArticulations(representative[i].first, representative[i].second, -1);
  }

  //find adjacencies amongst cut vertices
  for (int i = 0; i < num_blocks; i++) {
    int x = cut_location[i].first;
    int y = cut_location[i].second;

    if (getBlock(x,y+1) != -1) block_neighbors[i].insert(getBlock(x,y+1));
    if (getBlock(x,y-1) != -1) block_neighbors[i].insert(getBlock(x,y-1));
    if (getBlock(x-1,y) != -1) block_neighbors[i].insert(getBlock(x-1,y));
    if (getBlock(x+1,y) != -1) block_neighbors[i].insert(getBlock(x+1,y));
  }

  for (int i = 0; i < map_height; i++)
    for (int j = 0; j < map_width; j++)
      if (IsEmpty(i,j) && block_id[i][j] == -1) points.insert(make_pair(i,j));

  // found the cut vertices--now fill in the remainder of the data structures
  while (!points.empty()) {
    set<pair<int,int> >::iterator it = points.begin();
    int x = (*it).first, y = (*it).second;
    battlefront.push_back(false);
    block_neighbors.push_back(set<int>());
    blockDFS(x, y, num_blocks++);
    cut_location.push_back(make_pair(-1,-1));
  }

}

void Map::addCutVertex(int x, int y) {
  block_id[x][y] = num_blocks++;
  battlefront.push_back(false);
  block_neighbors.push_back(set<int>());
  cut_location.push_back(make_pair(x, y));
}

void Map::blockDFS(int x, int y, int block_idx) {
  block_id[x][y] = block_idx;
  points.erase(points.find(make_pair(x,y)));
  blockDFSHelper(x,y+1,block_idx);
  blockDFSHelper(x,y-1,block_idx);
  blockDFSHelper(x+1,y,block_idx);
  blockDFSHelper(x-1,y,block_idx);
}

 void Map::blockDFSHelper(int x, int y, int block_idx) {
  if (points.find(make_pair(x,y)) != points.end()) {
    blockDFS(x,y,block_idx);
  } else if (IsEmpty(x,y) && block_id[x][y] >= 0 && block_id[x][y] != block_idx) {
    block_neighbors[block_idx].insert(block_id[x][y]);
    block_neighbors[block_id[x][y]].insert(block_idx);
  }
 }

void Map::calculateArticulations(int x, int y, int parent) {
  //fprintf(stderr, "Calculating cut vertices for (%d, %d)\n", x, y);
  int nodenum = ++counter;
  low[x][y] = nodenum;
  num[x][y] = nodenum;
  int children=0;


  int wx, wy;

  wx = x;
  wy = y+1;
  if (IsEmpty(wx,wy)) {
    if (num[wx][wy] == -1) {
      children++;
      calculateArticulations(wx, wy, nodenum);
      if (low[wx][wy] >= nodenum && parent != -1) {
        addCutVertex(x,y);
      }
      if (low[wx][wy] < low[x][y]) low[x][y] = low[wx][wy];
    } else {
      if (num[wx][wy] < nodenum)
        if (num[wx][wy] < low[x][y]) low[x][y] = num[wx][wy];
    }
  }
  wx = x;
  wy = y-1;
  if (IsEmpty(wx,wy)) {
    if (num[wx][wy] == -1) {
      children++;
      calculateArticulations(wx, wy, nodenum);
      if (low[wx][wy] >= nodenum && parent != -1) {
        addCutVertex(x,y);
      }
      if (low[wx][wy] < low[x][y]) low[x][y] = low[wx][wy];
    } else {
      if (num[wx][wy] < nodenum)
        if (num[wx][wy] < low[x][y]) low[x][y] = num[wx][wy];
    }
  }
  wx = x+1;
  wy = y;
  if (IsEmpty(wx,wy)) {
    if (num[wx][wy] == -1) {
      children++;
      calculateArticulations(wx, wy, nodenum);
      if (low[wx][wy] >= nodenum && parent != -1) {
        addCutVertex(x,y);
      }
      if (low[wx][wy] < low[x][y]) low[x][y] = low[wx][wy];
    } else {
      if (num[wx][wy] < nodenum)
        if (num[wx][wy] < low[x][y]) low[x][y] = num[wx][wy];
    }
  }
  wx = x-1;
  wy = y;
  if (IsEmpty(wx,wy)) {
    if (num[wx][wy] == -1) {
      children++;
      calculateArticulations(wx, wy, nodenum);
      if (low[wx][wy] >= nodenum && parent != -1) {
        addCutVertex(x,y);
      }
      if (low[wx][wy] < low[x][y]) low[x][y] = low[wx][wy];
    } else {
      if (num[wx][wy] < nodenum)
        if (num[wx][wy] < low[x][y]) low[x][y] = num[wx][wy];
    }
  }

  if (parent == -1 && children > 1) {
    addCutVertex(x,y);
  }
}




bool Map::endGame() const {
  set<int> my_components;
  my_components.insert(component_id[player_one_x][player_one_y-1]);
  my_components.insert(component_id[player_one_x][player_one_y+1]);
  my_components.insert(component_id[player_one_x+1][player_one_y]);
  my_components.insert(component_id[player_one_x-1][player_one_y]);
  my_components.erase(-1);
  if (my_components.find(component_id[player_two_x][player_two_y-1]) != my_components.end()) return false;
  if (my_components.find(component_id[player_two_x][player_two_y+1]) != my_components.end()) return false;
  if (my_components.find(component_id[player_two_x+1][player_two_y]) != my_components.end()) return false;
  if (my_components.find(component_id[player_two_x-1][player_two_y]) != my_components.end()) return false;
  return true;
}

Map::Map(const Map &other, int player, string direction, bool computeScore) {

  player_one_x = other.MyX();
  player_one_y = other.MyY();
  player_two_x = other.OpponentX();
  player_two_y = other.OpponentY();
  map_width = other.Width();
  map_height = other.Height();
  is_wall = other.GetWalls();

  if (player==1) {
    is_wall[player_one_x][player_one_y] = true;
  } else {
    is_wall[player_two_x][player_two_y] = true;
  }

  int c = (int)direction[0];
  switch (c) {
    case 'n':
    case 'N':
      if (player == 1) {
        player_one_y--;
      } else {
        player_two_y--;
      }
      break;
    case 'e':
    case 'E':
      if (player == 1) {
        player_one_x++;
      } else {
        player_two_x++;
      }
      break;
    case 's':
    case 'S':
      if (player == 1) {
        player_one_y++;
      } else {
        player_two_y++;
      }
      break;
    default:
      if (player == 1) {
        player_one_x--;
      } else {
        player_two_x--;
      }
      break;
  }

  double start_time= CycleTimer::currentSeconds();
  computeComponents();
  double end_time = CycleTimer::currentSeconds();;
  
  if (computeScore) {
    computeBlocks();
    computeVaronoi();

    varonoiBlockScoreWrapper();
    // parallelVaronoiBlockScoreWrapper();
  }

}

int Map::Width() const {
  return map_width;
}

int Map::Height()  const {
  return map_height;
}

bool Map::IsEmpty(int x, int y) const {
  return !IsPlayer(x,y) && !IsWall(x,y);
}

bool Map::IsPlayer(int x, int y) const {
  if((x == player_one_x && y == player_one_y) || (x == player_two_x && y == player_two_y)) {
    return true;
  }
  else {
    return false;
  }
}

bool Map::IsInBounds(int x, int y) const {
  if (x < 0 || y < 0 || x >= map_width || y >= map_height) {
    return false;
  } else {
    return true;
  }
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
  // fprintf(stderr, "x: %d, y: %d, playerx: %d, playery: %d\n",x,y, MyX(), MyY());
  // fprintf(stderr, "x: %d, y: %d\n",MyX(),MyY());
  return block_id[x][y];
}

bool Map::blockBattlefront(int block_id) const {
  return battlefront[block_id];
}

int Map::blockVaronoi(int block_id, int player) const {
  return player == 1 ? my_block_varonoi[block_id] : opp_block_varonoi[block_id];
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

int Map::varonoiBlockScore(int block_id, std::vector<bool> visited, int player) {
  int block_score = blockVaronoi(block_id, player);

  // if(blockBattlefront(block_id)) return block_score;
  if (block_score==0) return 0;
  
  set<int> neighbor_blocks = neighborBlocks(block_id);
  set<int>::iterator it;

  int max_score = block_score;
  for(it = neighbor_blocks.begin(); it!=neighbor_blocks.end(); it++){
    int child_block = *it; 
    int total_score;
    if(!visited[child_block]) {
      visited[child_block] = true;
      int child_score = varonoiBlockScore(child_block, visited, player);
      visited[child_block] = false;
      total_score = block_score + child_score;
      max_score = total_score > max_score ? total_score : max_score; 
    }
  }

  return max_score;
}

void Map::varonoiBlockScoreWrapper() {
  vector<bool> my_visited(numBlocks(),false);
  vector<bool> opp_visited(numBlocks(),false);
  // printBlocks();

  int my_score=0, opp_score=0, new_score;
  int myX[4] = {player_one_x, player_one_x+1, player_one_x, player_one_x-1};
  int myY[4] = {player_one_y-1, player_one_y, player_one_y+1, player_one_y};
  int oppX[4] = {player_two_x, player_two_x+1, player_two_x, player_two_x-1};
  int oppY[4] = {player_two_y-1, player_two_y, player_two_y+1, player_two_y};
  for(int i=0; i<4; i++){
    int my_block_id = getBlock(myX[i],myY[i]);
    if(my_block_id >=0){
      my_visited[my_block_id] = true;
      new_score = varonoiBlockScore(my_block_id, my_visited, 1);
      my_visited[my_block_id] = false;

      my_score = MAX(my_score,new_score);
    }
    int opp_block_id = getBlock(oppX[i],oppY[i]);
    if(opp_block_id >=0){
      opp_visited[opp_block_id] = true;
      new_score = varonoiBlockScore(opp_block_id, opp_visited, 2);
      opp_visited[opp_block_id] = false;

      opp_score = MAX(opp_score,new_score);
    }

  }
  score = my_score - opp_score;
}

void Map::parallelVaronoiBlockScoreWrapper() {
  vector<bool> my_visited(numBlocks(),false);
  vector<bool> opp_visited(numBlocks(),false);

  cilk::reducer_max<int> my_score, opp_score;
  int new_score;

  int myX[4] = {player_one_x, player_one_x+1, player_one_x, player_one_x-1};
  int myY[4] = {player_one_y-1, player_one_y, player_one_y+1, player_one_y};
  int oppX[4] = {player_two_x, player_two_x+1, player_two_x, player_two_x-1};
  int oppY[4] = {player_two_y-1, player_two_y, player_two_y+1, player_two_y};

  cilk_for(int i=0; i<8; i++){
    if(i<4){
      int my_block_id = getBlock(myX[i],myY[i]);
      if(my_block_id >=0){
        my_visited[my_block_id] = true;
        new_score = varonoiBlockScore(my_block_id, my_visited, 1);
        my_visited[my_block_id] = false;

        my_score.calc_max(new_score);
      }
    } else {
      int opp_block_id = getBlock(oppX[i-4],oppY[i-4]);
      if(opp_block_id >=0){
        opp_visited[opp_block_id] = true;
        new_score = varonoiBlockScore(opp_block_id, opp_visited, 2);
        opp_visited[opp_block_id] = false;

        opp_score.calc_max(new_score);
      }
    }
  }

  // for(int i=0; i<4; i++){
  //   int my_block_id = getBlock(myX[i],myY[i]);
  //   if(my_block_id >=0){
  //     my_visited[my_block_id] = true;
  //     new_score = varonoiBlockScore(my_block_id, my_visited, 1);
  //     my_visited[my_block_id] = false;

  //     my_score.calc_max(new_score);
  //   }
  //   int opp_block_id = getBlock(oppX[i],oppY[i]);
  //   if(opp_block_id >=0){
  //     opp_visited[opp_block_id] = true;
  //     new_score = varonoiBlockScore(opp_block_id, opp_visited, 2);
  //     opp_visited[opp_block_id] = false;
  //     opp_score.calc_max(new_score);
  //   }
  // }

  score = my_score.get_value() - opp_score.get_value();
}