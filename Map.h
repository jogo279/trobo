// Map.h
//
// Handles the Tron map. Also handles communicating with the Tron game engine.
// You don't need to change anything in this file.

#include <string>
#include <vector>
#include <utility>
#include <set>

class Map {
 public:
  // Constructs a Map by reading an ASCII representation from the console
  // (stdin).
  Map();

  // Constructs a map by taking a base map, and applying a move to a player
  Map(const Map &other, int player, std::string direction);


  // Returns the width of the Tron map.
  int Width() const;

  // Returns the height of the Tron map.
  int Height() const;

  // Returns whether or not the given cell is a wall or not. TRUE means it's
  // a wall, FALSE means it's not a wall, and is passable. Any spaces that are
  // not on the board are deemed to be walls.
  bool IsWall(int x, int y) const;

  // Get my X and Y position. These are zero-based.
  int MyX() const;
  int MyY() const;

  // Get the opponent's X and Y position. These are zero-based.
  int OpponentX() const;
  int OpponentY() const;

  // Sends your move to the contest engine. Only the first character of
  // the string is used. It is case insensitive. The four possible moves are
  //   * "N" -- North. Negative Y direction.
  //   * "E" -- East. Positive X direction.
  //   * "S" -- South. Positive X direction.
  //   * "W" -- West. Negative X direction.
  // Other strings can be valid moves, too. For example: "North", "east",
  // "s", and "WwWwWest!" are all valid moves, because they start with one
  // of the four allowed characters.
  static void MakeMove(const std::string& move);

  // Sends your move to the contest engine. The four possible moves are
  //   * 1 -- North. Negative Y direction.
  //   * 2 -- East. Positive X direction.
  //   * 3 -- South. Positive X direction.
  //   * 4 -- West. Negative X direction.
  static void MakeMove(int move);

  std::vector< std::vector<bool> > GetWalls() const;

  int garbage() const;

  bool endGame() const; 

  int vertexScore(int i, int j);

  int numBlocks() const;
  int getBlock(int x, int y) const;
  bool blockBattlefront(int block_id) const;
  int blockVaronoi(int block_id) const;
  std::pair<int, int> cutVertex(int block_id) const;//returns (-1,-1) if not cut vertex
  std::set<int> neighborBlocks(int block_id) const;

  void printStats() const;

 private:
  void ReadFromFile(FILE *file_handle);


  void computeVaronoi();

  void computeBlocks();
  void calculateArticulations(int x, int y, int parent);
  void addCutVertex(int x, int y);
  void blockDFS(int x, int y, int block_idx);
  void blockDFSHelper(int x, int y, int block_idx);


  void findComponent(std::pair<int, int> point, int idx);
  void computeComponents();

 private:
  // Indicates whether or not each cell in the board is passable.
  std::vector<std::vector<bool> > is_wall;

  // The locations of both players.
  int player_one_x, player_one_y;
  int player_two_x, player_two_y;

  // Map dimensions.
  int map_width, map_height;

  // Data structures for components
  std::vector<std::vector<int> > component_id;
  std::vector<std::pair<int ,int> > representative;
  std::set<std::pair<int, int> > points;
  int num_components;

  // Data structures for blocks
  int num_blocks;
  std::vector<std::vector<int> > block_id;
  std::vector<bool> battlefront;
  std::vector<int> block_varonoi;
  std::vector< std::pair<int, int> > cut_location;
  std::vector< std::set<int> > block_neighbors;


  // Data for BlockDFS
  int counter;
  std::vector<std::vector<int> > num;
  std::vector<std::vector<int> > low;
  std::vector<std::vector<std::pair<int, int> > > parent;

  // Data structures for Varonoi
  std::set< std::pair<int, int> > my_set;
  std::set< std::pair<int, int> > my_set_new;
  std::set< std::pair<int, int> > opp_set;
  std::set< std::pair<int, int> > opp_set_new;
  std::vector<std::vector<bool> > grid;

};
