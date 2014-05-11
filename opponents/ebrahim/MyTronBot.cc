/****************************/
/*    In the name of God    */
/****************************/
/* Tron bot                 */
/****************************/
/* By ebrahim               */
/* http://ebrahim.ir        */
/* ebrahim at mohammadi.ir  */
/* License: public domain   */
/****************************/

#include <string>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <stdint.h>

#include <signal.h>
#include <sys/time.h>   // For setitimer

#include "Map.h"

#define MEMOIZE 1
#define LOG 1

#define TIMEOUT 990000    // usec
#define FIRST_TIMEOUT 2850000   // usec

#if MEMOIZE

#define CACHE_SIZE ((1<<19) + (1<<18))
#define KEEP_GAME_STATE 1
#define CACHE_RANDOM_DROP 0

#include <algorithm>

#define _BACKWARD_BACKWARD_WARNING_H 1    // Disable warning
#include <ext/hash_map>

#if !CACHE_RANDOM_DROP
#include <deque>
#endif

#endif

#if 0
FILE* set_file = fopen("set.txt", "w");
FILE* get_file = fopen("get.txt", "w");
#endif

static const int x_diff[4] = { 0, 1, 0, -1 };
static const int y_diff[4] = { -1, 0, 1, 0 };

static bool timed_out = false;

class AlphaBeta
{
public:
  enum { INFINITY = 1 << 30 };
#if MEMOIZE
  enum { START_DEPTH = 2 };
#else
  enum { START_DEPTH = 8 };
#endif
  enum { SCORE_LOSE = -INFINITY + 1 };
  enum { SCORE_DRAW = -1 };
  enum { SCORE_WIN = INFINITY - 1 };
  enum { SCORE_SEPARATED = 2 };
  enum { SCORE_UNSEPARATED = 1 };

#if MEMOIZE
  class GameState
  {
  public:
    GameState()
    : pos(0)
    , hash(0)
    {
      for (int xx = 0; xx < MAX_SIDE; ++xx)
        map[xx] = 0;
    }

    GameState(const AlphaBeta& alphabeta)
    : pos(0)
    , hash(0)
    {
      update_pos(alphabeta);
      for (int xx = 0; xx < AlphaBeta::width; ++xx)
      {
        uint64_t col = 0;
        for (int yy = AlphaBeta::height - 1; yy >= 0 ; --yy)
        {
          col <<= 1;
          col |= alphabeta.wall[xx][yy];
        }
        map[xx] = col;
        hash_column(xx);
      }
    }

#if LOG
    void print(FILE* file)
    {
      int x1 = (pos >> 24) & 0xFF;
      int y1 = (pos >> 16) & 0xFF;
      int x2 = (pos >> 8) & 0xFF;
      int y2 = pos & 0xFF;
      fprintf(file, "%.16llx: %d %d %d %d\n", (unsigned long long) hash, x1, y1, x2, y2);
      for (int yy = 0; yy < AlphaBeta::height ; ++yy)
      {
        for (int xx = 0; xx < AlphaBeta::width; ++xx)
        {
          if (xx == x1 && yy == y1)
            putc('1', file);
          else if (xx == x2 && yy == y2)
            putc('2', file);
          else if ((map[xx] >> yy) & 1)
            putc('#', file);
          else
            putc(' ', file);
        }
        putc('\n', file);
      }
    }
#endif

    void hash_column(uint8_t x)
    {
      hash ^= (map[x] << x) | (map[x] >> (64 - x));
    }

    void set(int x, int y, bool value)
    {
      hash_column(x);   // Remove old column from hash
      map[x] &= ~(uint64_t(1) << y);    // Reset bit to 0
      map[x] |= uint64_t(value) << y;   // Set it to value
      hash_column(x);   // Add new column to hash
    }

    void update_pos(const AlphaBeta& alphabeta)
    {
      hash ^= pos;    // Remove old pos from hash
      pos = alphabeta.x;
      pos <<= 8;
      pos |= alphabeta.y;
      pos <<= 8;
      pos |= alphabeta.enemy_x;
      pos <<= 8;
      pos |= alphabeta.enemy_y;
      pos |= pos << 32;
      hash ^= pos;    // Add new pos to hash
    }

    bool operator<(const GameState& other) const
    {
      for (int xx = 0; xx < AlphaBeta::width; ++xx)
      {
        if (map[xx] < other.map[xx])
          return true;
        if (map[xx] > other.map[xx])
          return false;
      }
      return pos < other.pos;
    }

    size_t operator()(const GameState& value) const   // Hash function
    {
      return value.hash;
    }

    bool operator()(const GameState& one, const GameState& two) const
    {
      if (one.pos != two.pos)
        return false;
      for (int xx = 0; xx < AlphaBeta::width; ++xx)
        if (one.map[xx] != two.map[xx])
          return false;
      return true;
    }

    uint64_t map[MAX_SIDE];
    uint64_t pos;
    uint64_t hash;
  };

  typedef __gnu_cxx::hash_map<GameState, int, GameState, GameState> EvaluationCache;
  //typedef std::map<GameState, int> EvaluationCache;
#if !CACHE_RANDOM_DROP
  typedef std::deque<EvaluationCache::iterator> EvaluationCacheAge;
#endif

  struct Heuristic
  {
    bool operator<(const Heuristic& other) const { return score < other.score; }
    int neighbor, score;
  };
#endif

  AlphaBeta(const Map& map)
  : x(map.my_x())
  , y(map.my_y())
  , enemy_x(map.opponent_x())
  , enemy_y(map.opponent_y())
  , full_search(false)
  {
    width = map.width;
    height = map.height;
    for (int xx = 0; xx < width; ++xx)
      for (int yy = 0; yy < height; ++yy)
        wall[xx][yy] = map.is_wall(xx, yy);
#if KEEP_GAME_STATE
    game_state = GameState(*this);
#endif
  }

  bool is_wall(int xx, int yy)
  {
    if (xx < 0 || yy < 0 || xx >= width || yy >= height)
      return true;
    return wall[xx][yy];
  }

  void set_wall(int xx, int yy, bool value)
  {
    if (xx < 0 || yy < 0 || xx >= width || yy >= height)
      return;
    wall[xx][yy] = value;
  }

  void swap_roles()
  {
    int tmp = x;
    x = enemy_x;
    enemy_x = tmp;
    tmp = y;
    y = enemy_y;
    enemy_y = tmp;
  }

  int run()
  {
    int move = 0;
    full_search = false;
    for (int depth = START_DEPTH; !full_search; depth += 2)
    {
      int best_neighbor;
      int best_depth;
      full_search = true;   // Assume full search, until game tree is cut
#if LOG
      total  = 0;
      miss = 0;
      eval = 0;
      int score = alphabeta(depth, -INFINITY, INFINITY, best_neighbor, best_depth);
#else
      alphabeta(depth, -INFINITY, INFINITY, best_neighbor, best_depth);
#endif
      if (timed_out)
        break;
#if LOG
      //fprintf(stderr, "depth: %d\n", depth);
      fprintf(stderr, "depth: %d,\ttotal: %d,\thit: %d,\teval: %d,\tbest depth: %d,\tdecision: %d,\tscore: %d\n",
          depth, total, total - miss, eval, best_depth, best_neighbor + 1, score);
#endif
      move = best_neighbor;
    }
    return move;
  }

  int alphabeta(int depth, int alpha, int beta, int& best_neighbor, int& best_depth)
  {
    best_neighbor = -1;
    best_depth = depth;
    if (timed_out)
      return -INFINITY;
    //fprintf(stderr, "Depth: %d, Alpha: %d, Beta: %d\n", depth, alpha, beta);
    if (depth % 2 == 0)   // If both players have moved
    {
      if (wall[x][y] || wall[enemy_x][enemy_y] || (x == enemy_x && y == enemy_y))   // If search in this branch ended
        return evaluate();
      else if (depth <= 0)
      {
        full_search = false;
        return evaluate();
      }
    }
    wall[x][y] = true;
#if MEMOIZE
#if KEEP_GAME_STATE
    game_state.set(x, y, true);
#endif
    Heuristic heuristics[4];
    for (int neighbor = 0; neighbor < 4; ++neighbor)
    {
      x += x_diff[neighbor];
      y += y_diff[neighbor];
      swap_roles();
#if KEEP_GAME_STATE
      game_state.update_pos(*this);
#endif
      heuristics[neighbor].neighbor = neighbor;
      heuristics[neighbor].score = depth % 2 ? evaluate(true) : -neighbor;
      swap_roles();
      x -= x_diff[neighbor];
      y -= y_diff[neighbor];
#if KEEP_GAME_STATE
      game_state.update_pos(*this);
#endif
    }
    std::sort(heuristics, heuristics + 4);
    for (int i = 0; i < 4; ++i)
    {
      int neighbor = heuristics[i].neighbor;
#else
    for (int neighbor = 0; neighbor < 4; ++neighbor)
    {
#endif
      x += x_diff[neighbor];
      y += y_diff[neighbor];
      swap_roles();
#if KEEP_GAME_STATE
      game_state.update_pos(*this);
#endif
      int child_best_neighbor, child_best_depth;
      int new_alpha = -alphabeta(depth - 1, -beta, -alpha, child_best_neighbor, child_best_depth);
      swap_roles();
      x -= x_diff[neighbor];
      y -= y_diff[neighbor];
#if KEEP_GAME_STATE
      game_state.update_pos(*this);
#endif
      //fprintf(stderr, "depth: %d, my_max_score: %d, alpha: %d, beta: %d\n", depth, my_max_score, alpha, beta);
      if (new_alpha > alpha || (new_alpha == alpha && alpha == SCORE_LOSE && child_best_depth < best_depth))
      {
        alpha = new_alpha;
        best_neighbor = neighbor;
        best_depth = child_best_depth;
      }
      if (beta <= alpha && depth % 2)   // Beta cut-off
        break;
    }
    wall[x][y] = false;
#if KEEP_GAME_STATE
    game_state.set(x, y, false);
#endif
#if MEMOIZE
    if (depth % 2 == 0 && !timed_out)
      cache_score(alpha);
#endif
    return alpha;
  }

#if MEMOIZE
  void cache_score(int score)
  {
#if !KEEP_GAME_STATE
    GameState game_state(*this);
#endif
    EvaluationCache::iterator cached = cache.find(game_state);
#if CACHE_RANDOM_DROP
    if (cached == cache.end())
      cache.insert(EvaluationCache::value_type(game_state, score));
    else
      cached->second = score;
#else
    if (cached == cache.end())
    {
#if 0
      game_state.print(set_file);
      fprintf(set_file, "%+d\n", score);
#endif
      std::pair<EvaluationCache::iterator, bool> res = cache.insert(EvaluationCache::value_type(game_state, score));
      cache_age.push_back(res.first);
      if (cache_age.size() >= CACHE_SIZE)
      {
        cache.erase(cache_age.front());
        cache_age.pop_front();
      }
    }
    else
      cached->second = score;
#endif
  }
#endif

  int floodfill(int xx, int yy, int& depth, int& enemy_distance)
  {
    struct Pos
    {
      Pos() { }
      Pos(int x, int y) : x(x), y(y) { }
      int x, y;
    };

    static int reached[MAX_SIDE][MAX_SIDE];
    static Pos neighbors[MAX_SIDE * MAX_SIDE];

    depth = 0;
    enemy_distance = INFINITY;
    for (int xxx = 0; xxx < width; ++xxx)
      for (int yyy = 0; yyy < height; ++yyy)
        reached[xxx][yyy] = 0;
    int tail = 0;
    int head = 0;
    reached[xx][yy] = 1;
    neighbors[tail++] = Pos(xx, yy);
    int density = 0;    // Neighborhood density score
    do
    {
      int x = neighbors[head].x;
      int y = neighbors[head].y;
      int d = reached[x][y];
      int open_neighbors = 0;
      for (int diff = 0; diff < 4; ++diff)
      {
        int xx = x + x_diff[diff];
        int yy = y + y_diff[diff];
        if (wall[xx][yy])
          continue;
        ++open_neighbors;
        if (reached[xx][yy])
          continue;
        depth += d;
        reached[xx][yy] = d + 1;
        neighbors[tail++] = Pos(xx, yy);
      }
      if (open_neighbors > 2)
        density += 10;
      else if (open_neighbors > 1)
        density += 9;
      ++head;   // Pop
    } while (head != tail);
    for (int diff = 0; diff < 4; ++diff)
    {
      int xx = enemy_x + x_diff[diff];
      int yy = enemy_y + y_diff[diff];
      int d = reached[xx][yy];
      if (d && d < enemy_distance)
        enemy_distance = d;
    }
    return density;
  }

  int distance(int x1, int y1, int x2, int y2)
  {
    int dx = x1 - x2;
    int dy = y1 - y2;
    return dx * dx + dy * dy;
  }

#if MEMOIZE
  int evaluate(bool cheap = false)
#else
  int evaluate()
#endif
  {
#if 0
    fputs("-------------------------------\n", stderr);
    fprintf(stderr, "(x1,y1): (%d,%d)\n(x2,y2): (%d,%d)\n", x, y, enemy_x, enemy_y);
    for (int yy = 0; yy < height; ++yy)
    {
      for (int xx = 0; xx < width; ++xx)
        putc(xx == x && yy == y ? '1' : xx == enemy_x && yy == enemy_y ? '2' : wall[xx][yy] ? '#' : ' ', stderr);
      putc('\n', stderr);
    }
    fputs("-------------------------------\n", stderr);
#endif
    if (wall[x][y])   // If hit wall
    {
      if (wall[enemy_x][enemy_y])   // If enemy also hit wall
        return SCORE_DRAW;
      else
        return SCORE_LOSE;
    }
    else if (wall[enemy_x][enemy_y])    // If enemy hit wall
      return SCORE_WIN;
    if (x == enemy_x && y == enemy_y)   // If collided
      return SCORE_DRAW;

#if MEMOIZE
    // Memoize
#if !KEEP_GAME_STATE
    GameState game_state(*this);
#endif
#if LOG
    if (cheap)
      ++total;
#endif
    EvaluationCache::iterator cached = cache.find(game_state);
    if (cached != cache.end())    // If already calculated
      return cached->second;    // Return cached value
#if LOG
    if (cheap)
    {
      //game_state.print(get_file);
      ++miss;
    }
#endif
    if (cheap)
      return 0;
#if LOG
    ++eval;
#endif
#endif

    wall[x][y] = true;
    wall[enemy_x][enemy_y] = true;

    int our_distance = INFINITY;
    int my_flood_depth = INFINITY, your_flood_depth = 0;
    // Order is important because of our_distance
    int your_neighborhood = floodfill(enemy_x, enemy_y, your_flood_depth, our_distance);
    int my_neighborhood = floodfill(x, y, my_flood_depth, our_distance);

    wall[x][y] = false;
    wall[enemy_x][enemy_y] = false;

    int score = 0;
    if (our_distance == INFINITY)   // If separated
    {
      score += my_neighborhood;   // Prefer larger neighborhood for myself
      score -= your_neighborhood;   // Prefer smaller neighborhood for enemy
      score *= SCORE_SEPARATED;
    }
    else    // If in the same area
    {
      //score -= our_distance;    // Prefer near enemy
      score -= SCORE_UNSEPARATED * my_flood_depth;    // Prefer myself at center
      score += SCORE_UNSEPARATED * your_flood_depth;    // Prefer enemy at corners
    }
    //fprintf(stderr, "%d %d %d %d %d %d\n", max_neighbor_area_me, max_neighbor_area_enemy, enemy_distance, flood_depth_me, flood_depth_enemy, score);
#if MEMOIZE
    cache_score(score);
#endif
    return score;
  }

  static int width, height;
#if MEMOIZE
  static EvaluationCache cache;
#if !CACHE_RANDOM_DROP
  static EvaluationCacheAge cache_age;
#endif
#endif

  bool wall[MAX_SIDE][MAX_SIDE];
#if KEEP_GAME_STATE
  GameState game_state;
#endif
  int x, y;
  int enemy_x, enemy_y;
  int max_neighbor;
  bool full_search;
#if LOG
  int total, miss, eval;
#endif
};

int AlphaBeta::width = 0;
int AlphaBeta::height = 0;
#if MEMOIZE
AlphaBeta::EvaluationCache AlphaBeta::cache(CACHE_SIZE);
#if !CACHE_RANDOM_DROP
AlphaBeta::EvaluationCacheAge AlphaBeta::cache_age;
#endif
#endif

void timeout_handler(int /*sig*/)
{
  timed_out = true;
}

int main()
{
  signal(SIGALRM, timeout_handler);
  for (long timeout = FIRST_TIMEOUT; ; timeout = TIMEOUT)
  {
    Map map;
    timed_out = false;
    // Setup timer
    itimerval timer = { { 0, 0 }, { timeout / 1000000, timeout % 1000000} };
    setitimer(ITIMER_REAL, &timer, NULL);
    AlphaBeta alphabeta(map);
    int move = alphabeta.run() + 1;
    // Disable timer
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
    Map::make_move(move);
#if MEMOIZE && CACHE_RANDOM_DROP
    if (AlphaBeta::cache.size() >= CACHE_SIZE)
    {
      AlphaBeta::EvaluationCache::iterator begin = AlphaBeta::cache.begin();
      for (int i = 0; i < CACHE_SIZE / 2; ++i)    // Remove half of cache randomly
        ++begin;
      AlphaBeta::cache.erase(begin, AlphaBeta::cache.end());
    }
#endif
  }
  return 0;
}