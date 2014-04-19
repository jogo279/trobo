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
  computeBlocks();
  computeVaronoi();
}

void Map::printStats() const {
  fprintf(stderr, "Stats!\n");
  fprintf(stderr, "Player 1 (%d, %d); Player 2 (%d, %d)\n", player_one_x, player_one_y, player_two_x, player_two_y);
  fprintf(stderr, "End Game:%d\n", endGame());
  fprintf(stderr, "Num Blocks:%d\n", numBlocks());
  for (int i = 0; i < numBlocks(); i++) {
    fprintf(stderr, "Varonoi for block %d is %d\n", i, blockVaronoi(i));
    fprintf(stderr, "Block %d battlefront status: %d\n", i, blockBattlefront(i));
  }
  fprintf(stderr, "\n\n");
}



void Map::computeVaronoi() {
  grid = GetWalls();
  int x, y;

  

  block_varonoi = vector<int>(num_blocks, 0);

  //my_set and opp_set are essentially the "frontiers" of two simultaneous BFSs
  if (!IsWall(player_one_x, player_one_y))
    my_set.insert(make_pair(player_one_x, player_one_y));
  if (!IsWall(player_two_x, player_two_y))
    opp_set.insert(make_pair(player_two_x, player_two_y));


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
          my_set_new.insert(make_pair(newX[i], newY[i]));

          block_varonoi[getBlock(newX[i], newY[i])] ++;
          if (block_varonoi[getBlock(newX[i], newY[i])] < 0) battlefront[getBlock(newX[i], newY[i])] = true;
        }
      }

      // if (!IsWall(x, y-1) && !grid[x][y-1]) {
      //   my_set_new.insert(make_pair(x, y-1));
      //   grid[x][y-1] = true;
      //   // block_varonoi[getBlock(x,y-1)] ++;
      //   std::cout << x,y-1 << getBlock(x,y-1) << "\n";
      // }
      // if (!IsWall(x+1, y) && !grid[x+1][y]) {
      //   my_set_new.insert(make_pair(x+1, y));
      //   grid[x+1][y] = true;
      //   // block_varonoi[getBlock(x+1,y)] ++;
      //   std::cout << x+1,y << getBlock(x+1,y) << "\n";
      // }
      // if (!IsWall(x, y+1) && !grid[x][y+1]) {
      //   my_set_new.insert(make_pair(x, y+1));
      //   grid[x][y+1] = true;
      //   // block_varonoi[getBlock(x,y+1)] ++;
      //   std::cout << x,y+1 << getBlock(x,y+1) << "\n";
      // }
      // if (!IsWall(x-1, y) && !grid[x-1][y]) {
      //   my_set_new.insert(make_pair(x-1, y));
      //   grid[x-1][y] = true;
      //   // block_varonoi[getBlock(x-1,y)] ++;
      //   std::cout << x-1,y << getBlock(x-1,y) << "\n";
      // }
    }

    // add neighboring squares to opp_set_new
    for (set <pair<int, int> >::iterator it = opp_set.begin(); it != opp_set.end(); ++it) {
      x = (*it).first;
      y = (*it).second;

      int newX[4] = {x, x+1, x, x-1};
      int newY[4] = {y-1, y, y+1, y};

      for(int i=0; i<4; i++){
        if (!IsWall(newX[i], newY[i]) && !grid[newX[i]][newY[i]]) {
          opp_set_new.insert(make_pair(newX[i], newY[i]));

          block_varonoi[getBlock(newX[i], newY[i])] --;
          if (block_varonoi[getBlock(newX[i], newY[i])] > 0) battlefront[getBlock(newX[i], newY[i])] = true;
        }
      }

      // if (!IsWall(x, y-1) && !grid[x][y-1]) {
      //   opp_set_new.insert(make_pair(x, y-1));
      //   grid[x][y-1] = true;
      //   // block_varonoi[getBlock(x,y-1)] --;
      //   std::cout << x,y-1 << getBlock(x,y-1) << "\n";
      // }
      // if (!IsWall(x+1, y) && !grid[x+1][y]) {
      //   opp_set_new.insert(make_pair(x+1, y));
      //   grid[x+1][y] = true;
      //   // block_varonoi[getBlock(x+1,y)] --;
      //   std::cout << x+1,y << getBlock(x+1,y) << "\n";
      // }
      // if (!IsWall(x, y+1) && !grid[x][y+1]) {
      //   opp_set_new.insert(make_pair(x, y+1));
      //   grid[x][y+1] = true;
      //   // block_varonoi[getBlock(x,y+1)] --;
      //   std::cout << x,y+1 << getBlock(x,y+1) << "\n";
      // }
      // if (!IsWall(x-1, y) && !grid[x-1][y]) {
      //   opp_set_new.insert(make_pair(x-1, y));
      //   grid[x-1][y] = true;
      //   // block_varonoi[getBlock(x-1,y)] --;
      //   std::cout << x-1,y << getBlock(x-1,y) << "\n";
      // }
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
        ++it;
      }
    }

    for (set <pair<int, int> >::iterator it = opp_set_new.begin(); it != opp_set_new.end(); it++) {
      grid[(*it).first][(*it).second] = true;
    }

    //update scores and sets
    my_set = my_set_new;
    opp_set = opp_set_new;

  }
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
      if (!IsWall(i,j)) points.insert(make_pair(i,j));

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


  for (int i = 0; i < map_height; i++)
    for (int j = 0; j < map_width; j++)
      if (!IsWall(i,j) && block_id[i][j] == -1) points.insert(make_pair(i,j));

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
  } else if (!IsWall(x,y) && block_id[x][y] >= 0 && block_id[x][y] != block_idx) {
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
  if (!IsWall(wx, wy)) {
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
  if (!IsWall(wx, wy)) {
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
  if (!IsWall(wx, wy)) {
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
  if (!IsWall(wx, wy)) {
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

bool Map::blockBattlefront(int block_id) const {
  return battlefront[block_id];
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
