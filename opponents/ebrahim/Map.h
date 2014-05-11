#ifndef _MAP_HPP_
#define _MAP_HPP_

// Map.h
//
// Handles the Tron map. Also handles communicating with the Tron game engine.
// You don't need to change anything in this file.

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define MAX_SIDE 64

enum { NORTH = 1, EAST = 2, SOUTH = 3, WEST = 4 };

class Map
{
public:
  // Constructs a Map by reading an ASCII representation from the console
  // (stdin).
  Map()
  {
    read_from_file(stdin);
  }

  // Returns whether or not the given cell is a wall or not. TRUE means it's
  // a wall, FALSE means it's not a wall, and is passable. Any spaces that are
  // not on the board are deemed to be walls.
  bool is_wall(int x, int y) const
  {
    if (x < 0 || y < 0 || x >= width || y >= height)
      return true;
    else
      return map[x][y];
  }

  bool is_wall(int move) const
  {
    switch (move)
    {
      case NORTH:
        return is_wall(player_one_x, player_one_y - 1);
      case EAST:
        return is_wall(player_one_x + 1, player_one_y);
      case SOUTH:
        return is_wall(player_one_x, player_one_y + 1);
      case WEST:
        return is_wall(player_one_x - 1, player_one_y);
    }
    return true;
  }

  // Get my X and Y position. These are zero-based.
  int my_x() const
  {
    return player_one_x;
  }

  int my_y() const
  {
    return player_one_y;
  }

  // Get the opponent's X and Y position. These are zero-based.
  int opponent_x() const
  {
    return player_two_x;
  }

  int opponent_y() const
  {
    return player_two_y;
  }

  // Sends your move to the contest engine. The four possible moves are
  //   * 1 -- North. Negative Y direction.
  //   * 2 -- East. Positive X direction.
  //   * 3 -- South. Positive X direction.
  //   * 4 -- West. Negative X direction.
  static void make_move(int move)
  {
    fprintf(stdout, "%d\n", move);
    fflush(stdout);
  }

  // Map dimensions.
  int width, height;

private:
  // Load a board from an open file handle. To read from the console, pass
  // stdin, which is actually a (FILE*).
  //   file_handle -- an open file handle from which to read.
  //
  // If there is a problem, the function returns NULL. Otherwise, a valid
  // Board structure is returned.
  //
  // The file should be an ascii file. The first line contains the width and
  // height of the board, separated by a space. subsequent lines contain visual
  // representations of the rows of the board, using '#' and space characters.
  // The starting positions of the two players are indicated by '1' and '2'
  // characters. There must be exactly one '1' character and one '2' character
  // on the board. For example:
  // 6 4
  // ######
  // #1# 2#
  // #   ##
  // ######
  void read_from_file(FILE *file_handle)
  {
    int x, y, c;
    int num_items = fscanf(file_handle, "%d %d\n", &width, &height);
    if (feof(file_handle) || num_items < 2)
      exit(0);    // End of stream means end of game. Just exit.
    memset(map, 0, sizeof(map));
    x = 0;
    y = 0;
    while (y < height && (c = fgetc(file_handle)) != EOF)
    {
      switch (c)
      {
        case '\r':
          break;
        case '\n':
          if (x != width)
          {
            fprintf(stderr, "x != width in Board_ReadFromStream\n");
            return;
          }
          ++y;
          x = 0;
          break;
        case '#':
          if (x >= width)
          {
            fprintf(stderr, "x >= width in Board_ReadFromStream\n");
            return;
          }
          map[x][y] = true;
          ++x;
          break;
        case ' ':
          if (x >= width)
          {
            fprintf(stderr, "x >= width in Board_ReadFromStream\n");
            return;
          }
          map[x][y] = false;
          ++x;
          break;
        case '1':
          if (x >= width)
          {
            fprintf(stderr, "x >= width in Board_ReadFromStream\n");
            return;
          }
          map[x][y] = false;
          player_one_x = x;
          player_one_y = y;
          ++x;
          break;
        case '2':
          if (x >= width)
          {
            fprintf(stderr, "x >= width in Board_ReadFromStream\n");
            return;
          }
          map[x][y] = false;
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

private:
  // Indicates whether or not each cell in the board is passable.
  bool map[MAX_SIDE][MAX_SIDE];

  // The locations of both players.
  int player_one_x, player_one_y;
  int player_two_x, player_two_y;
};

#endif // _MAP_HPP_