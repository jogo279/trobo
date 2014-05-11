#!/usr/bin/python
# NEXT:
#   only sub components that were used in block
#   use cut points in eval to see if opponent is cut off,
#   use cut points in eval to see if own space is cut off

import random, sys, time
from operator import add, neg

import logging
log = logging.getLogger("")

def info(fmt,*args):
    logging.info(fmt,*args)
def debug(fmt,*args):
    logging.info(fmt,*args)    

NORTH = 1
EAST  = 2
SOUTH = 3
WEST  = 4

FLOOR = ' '
WALL  = '#'
ME    = '1'
THEM  = '2'

DIRECTIONS = (NORTH, EAST, SOUTH, WEST)

# If you want to use this file, make sure to include it in your
# submission. You may modify it and submit the modified copy, or you
# may discard it and roll your own.

"""
Provided code for the Python starter package

See the example bots randbot.py and wallbot.py to get started.
"""

import sys, os

def invalid_input(message):
    """You do not need to call this function directly."""

    print >>sys.stderr, "Invalid input: %s" % message
    sys.exit(1)

def readline(buf):
    """You do not need to call this function directly."""

    while not '\n' in buf:
        tmp = os.read(0, 1024)
        if not tmp:
            break
        buf += tmp

    if not buf.strip():
        return None, buf
    if not '\n' in buf:
        invalid_input('unexpected EOF after "%s"' % buf)

    index = buf.find('\n')
    line = buf[0:index]
    rest = buf[index + 1:]
    return line, rest

class TBoard(object):
    """The Tron Board.

    The recommended way to use this class is as follows:

        def which_move(board):
            # figure this part out yourself
            return tron.NORTH

        for board in tron.Board.generate():
            tron.move(which_move(board))

    Feel free to add stuff to this class.
    """

    def __init__(self, width, height, board):
        """You do not need to call this method directly."""

        self.board = board
        self.height = height
        self.width = width
        self._me = None
        self._them = None

    @staticmethod
    def read(buf):
        """You do not need to call this method directly."""

        meta, buf = readline(buf)

        if not meta:
            return None, buf

        dim = meta.split(' ')

        if len(dim) != 2:
            invalid_input("expected dimensions on first line")

        try:
            width, height = int(dim[0]), int(dim[1])
        except ValueError:
            invalid_input("malformed dimensions on first line")

        lines = []

        while len(lines) != height:
            line, buf = readline(buf)
            if not line:
                invalid_input("unexpected EOF reading board")
            lines.append(line)

        board = [line[:width] for line in lines]

        if len(board) != height or any(len(board[y]) != width for y in xrange(height)):
            invalid_input("malformed board")

        return TBoard(width, height, board), buf

    @staticmethod
    def generate():
        """Generate board objects, once per turn.

        This method returns a generator which you may iterate over.
        Make sure to call tron.move() exactly once for every board
        generated, or your bot will not work.
        """

        buf = ''

        while True:
            board, buf = TBoard.read(buf)
            if not board:
                break
            yield board

        if buf.strip():
            invalid_input("garbage after last board: %s" % buf)

    def __getitem__(self, coords):
        """Retrieve the object at the specified coordinates.

        Use it like this:

            if board[3, 2] == tron.THEM:
                # oh no, the other player is at (3,2)
                run_away()

        Coordinate System:
            The coordinate (y, x) corresponds to row y, column x.
            The top left is (0, 0) and the bottom right is
            (board.height - 1, board.width - 1). Out-of-range
            coordinates are always considered walls.

        Items on the board:
            tron.FLOOR - an empty square
            tron.WALL  - a wall or trail of a bot
            tron.ME    - your bot
            tron.THEM  - the enemy bot
        """

        y, x = coords
        if not 0 <= x < self.width or not 0 <= y < self.height:
            return WALL
        return self.board[y][x]

    def me(self):
        """Finds your position on the board.

        It is always true that board[board.me()] == tron.ME.
        """

        if not self._me:
            self._me = self.find(ME)
        return self._me

    def them(self):
        """Finds the other player's position on the board.

        It is always true that board[board.them()] == tron.THEM.
        """

        if not self._them:
            self._them = self.find(THEM)
        return self._them

    def find(self, obj):
        """You do not need to call this method directly."""

        for y in xrange(self.height):
            for x in xrange(self.width):
                if self[y, x] == obj:
                    return y, x
        raise KeyError("object '%s' is not in the board" % obj)

    def passable(self, coords):
        """Determine if a position in the board is passable.

        You can only safely move onto passable tiles, and only
        floor tiles are passable.
        """

        return self[coords] == FLOOR

    def rel(self, direction, origin=None):
        """Calculate which tile is in the given direction from origin.

        The default origin is you. Therefore, board.rel(tron.NORTH))
        is the tile north of your current position. Similarly,
        board.rel(tron.SOUTH, board.them()) is the tile south of
        the other bot's position.
        """

        if not origin:
            origin = self.me()
        y, x = origin
        if direction == NORTH:
            return y - 1, x
        elif direction == SOUTH:
            return y + 1, x
        elif direction == EAST:
            return y, x + 1
        elif direction == WEST:
            return y, x - 1
        else:
            raise KeyError("not a valid direction: %s" % direction)

    def adjacent(self, origin):
        """Calculate the four tiles that are adjacent to origin.

        Particularly, board.adjacent(board.me()) returns the four
        tiles to which you can move to this turn. This does not
        return tiles diagonally adjacent to origin.
        """

        return [self.rel(dir, origin) for dir in DIRECTIONS]

    def moves(self):
        """Calculate which moves are safe to make this turn.

        Any move in the returned list is a valid move. There
        are two ways moving to one of these tiles could end
        the game:

            1. At the beginning of the following turn,
               there are no valid moves off this tile.
            2. The other player also moves onto this tile,
               and you collide.
        """
        possible = dict((dir, self.rel(dir)) for dir in DIRECTIONS)
        passable = [dir for dir in possible if self.passable(possible[dir])]
        if not passable:
            # it seems we have already lost
            return [NORTH]
        return passable

def move(direction):
    print  direction
    sys.stdout.flush()

## Start of my code
    

BIG = 99999
WIN = 9999
MAXDEPTH = 100

def showGrid(data,width,height,fmt="%3d "):
    out = []
    out.append("".join(fmt % x for x in xrange(width)))
    for y in xrange(height):
        tmp = "".join([fmt % x for x in data[y*width:((y+1)*width-1)]])
        tmp += " %3d" % y
        out.append(tmp)
    return "\n".join(out)

def getScore(even,odd,startOdd):
    if startOdd:
        if  even > odd:
            return 2*odd + 1
        else:
            return 2*odd
    else:
        return getScore(odd, even, True)


def makeGrid(size, value):
    return [value for x in xrange(size)]

class TimeOut(Exception):
    pass

class Board():
    def __init__(self, board):
        self.width = 2 * (board.width / 2) + 1
        self.height = board.height
        self.data = [board[(y,x)] for y in xrange(self.height)
                     for x in xrange(self.width)]
        self.stack = []
        self.me = self.toN(board.me())
        self.them = self.toN(board.them())
        self.toMove = '1'
        width = self.width
        height = board.height
        self.key = "".join(self.data)
        def toN(yx):
            y,x = yx
            return y*width + x

        self.neighbors = [tuple([toN(n) for n in board.adjacent(p)
                           if board[n] != WALL])
                          for p in ((y,x) for y in xrange(height)
                                    for x in xrange(width))]

    def toN(self, yx):
        y,x = yx
        return y*self.width + x        
    def fromN(self, n):
        y,x = n / self.width, n % self.width
        return y,x
    def moves(self, who = None):
        if not who:
            who = self.toMove
        if who == '1':
            pos = self.me
            other = self.them
        else:
            pos = self.them
            other = self.me

        oy,ox = self.fromN(other)
        # db("%s %s %s %s" %(pos,other,self.fromN(pos), self.fromN(other)))
        def dist(n):
            y,x = self.fromN(n)
            dx = ox - x
            dy = oy - y
            return dx * dx + dy * dy
        r = [(dist(p),p) for p in self.adjacentT(pos)]
        # db("r: %s %s" % ((oy,ox), r))
        r.sort()
        return [p for (jj,p) in r]
       
            
    def __getitem__(self,yx):
        return self.data[yx]

    def isTerminal(self,depth=0):
        TIE = -0.5
        if self.me == self.them: return True, TIE
        myDone = len(self.moves('1')) == 0
        theirDone = len(self.moves('2')) == 0

        if myDone or theirDone:
            if myDone and theirDone: return True,TIE
            if theirDone: return True, WIN + depth
            if myDone: return True, - WIN - depth

        return False, 0
    
    def __setitem__(self, yx, v):
        self.data[yx] = v

    def adjacent(self, yx):
        return list(self.neighbors[yx])

    def adjacentT(self, yx):
        return [x for x in self.neighbors[yx] if self.data[x] == FLOOR]
    

    def __repr__(self):
        out = []
        for y in xrange(self.height):
            tmp = "".join(self.data[y*self.width + x] for x in xrange(self.width))
            out.append(tmp)
        return "\n".join(out)

    def move(self, pos):
        if self.toMove == '1':
            x = self.me
        else:
            x = self.them

        if self.toMove == '1':
            self.stack.append((pos, self.me))
            self.toMove = '2'
            
        else:
            me, oldMe = self.stack[-1]
            self[oldMe] = '#'
            self.stack.append((pos, self.them))
            self[me] = '1'
            self[pos] = '2'
            self[self.them] = '#'
            self.them = pos
            self.me = me
            self.toMove = '1'            
        self.key = "".join(self.data)
    def pop(self):
        (new,old) = self.stack[-1]
        self.stack = self.stack[:-1]
        self[new] = FLOOR
        if self.toMove == '1':
            newme,oldme = self.stack[-1]
            self[old] = '2'
            self.them = old
            self[newme] = FLOOR
            self.me = oldme
            self.toMove = '2'
        else:
            self[old] = '1'
            self.me = old
            self.toMove = '1'
        self.key = "".join(self.data)

def DFS(board, root):
    visited = makeGrid(board.width * board.height, -1)
    stack = board.adjacent(root)
    count = 0
    while stack:
        v = stack.pop()
        visited[v] = count
        count += 1
        for w in board.adjacentT(v):
            if visited[w] < 0:
                stack.append(w)
    return visited, count


def showList(xs):  return [(x,i) for (i,x) in enumerate(xs) if x > -1]


    
def assign_nums(board, root):
    graph = board.neighbors
    order = makeGrid(board.width * board.height, -1)
    parents = makeGrid(board.width * board.height, -1)
    parents[root] = root
    stack = [root]
    count = 0
    while stack:
        v = stack.pop()
        if order[v] < 0:
            order[v] = count
            count += 1
            for w in board.adjacentT(v):
                #             if board[w] != FLOOR and w != root: continue
                if order[w] < 0:
                    parents[w] = v
                    stack.append(w)
    return order, parents, count

def assign_low(board, root, order, parents):
    graph = board.neighbors
    
    low = makeGrid(board.width * board.height, -1)

    revNum = makeGrid(board.width * board.height, -1)

    
    count = max(order)
    for i,n in enumerate(order):
        if n > -1:
            revNum[n] = i
    cuts = []
    rootIsCut = len([w for w in board.adjacentT(root)
                     if parents[w] == root]) > 1
    
    for x in xrange(count,-1,-1):
        v = revNum[x]
        if order[v] != x:
            print v, x, order[v], revNum[x]
            raise "blah"
        low[v] = x

        for w in board.adjacentT(v):
            if parents[w] == v:
                if (low[w] >= x and v != root) or (v == root and rootIsCut):
                    cuts.append((v,w))
                low[v] = min(low[v], low[w])

            else:
                low[v] = min(low[v], order[w])
    component = assign_components(board, root, order, parents, low,cuts)
    return cuts, low, component
                
def assign_components(board, root, order, parents, low,cuts):
    cutSet = set(cuts)
    component = makeGrid(board.width * board.height, 0)
    count = max(order)
    revNum = makeGrid(board.width * board.height, -1)    

    for i,n in enumerate(order):
        if n > -1:
            revNum[n] = i

    rootIsCut = len([w for w in board.adjacentT(root)
                     if parents[w] == root]) > 1
    numComponents = 1
    component[root] = 1
    for x in xrange(count+1):
        v = revNum[x]
        for w in board.adjacent(v):
            if parents[w] != v: continue
            if (v,w) in cutSet:
                numComponents += 1
                component[w] = numComponents
            else:
                component[w] = component[v]
    return component


def countCuts(graph,root,parents,cuts):
    cutOrigins = set(x for (x,y) in cuts)
    cutDict = {}
    for x,y in cuts:
        # print x,y
        k = cutDict.get(x,[])
        # print k
        k.append(y)
        cutDict[x] = k

    def countFromCut(node):
        count = 1
        stack = [node]
        cuts = [0]
        while stack:
            v = stack.pop()
            for w in graph[v]:
                if parents[w] == v:
                    if v in cutDict and w in cutDict[v]:
                        cuts.append(countFromCut(w))
                    else:
                        count += 1
                        stack.append(w)
        score = count + max(cuts)
        return score
    tmp = [countFromCut(x) for x in graph[root]
                   if parents[x] == root]
    tmp.append(0)
    return max(tmp)

def isConnected(board, root, dest):
    data,junk = DFS(board, root)
    return any(data[y] > -1 for y in board.adjacent(dest))

def do_shortest_paths(board, yx):
    data = makeGrid(board.width*board.height, BIG)
    queue = [yx]
    d = data
    d[yx] = 0
    dist = 0
    while queue:
        nextQueue = []
        dist = dist + 1
        for p in queue:
            for n in board.adjacentT(p):
                if d[n] > dist:
                    d[n] = dist
                    nextQueue.append(n)
        queue = nextQueue
    return data

def discore(node,root):
    keep = node[root]
    node[root] = ' '
    order, parents, count = assign_nums(node, root) 
    cuts, low,comps = assign_low(node, root, order, parents)
    sc = countCuts(node.neighbors, root, parents, cuts)
    node[root] = keep
    return sc

def disconnectedsearch(node, depth,root,alpha,endTime):
    keep = node[root]
    data = node
    
    def go(node, depth, root, alpha):
        if time.time() > endTime:  raise TimeOut()            
        if depth == 0:
            sc = discore(node, root)
            return sc, None
        best = 0
        bestMove = None
        for mv in node.adjacentT(root):
            node[mv] = 'X'
            sc, junk = go(node, depth-1, mv, alpha-1)
            node[mv] = ' '
            if sc + 1 > best:
                best = sc + 1
                bestMove = mv
                if best >= alpha: break
        return best, bestMove
    return go(node, depth, root, alpha)


class Vor():
    def __init__(self):
        self.evals = 0
    def f(self, board,depth):
        # to score a general position, I need to modify the board.
        TIE = -0.5
        self.evals += 1
        me = board.me
        them = board.them
        connected = False
        if me == them: return TIE
        boarddata = board.data
        myDone = len(board.moves('1')) == 0
        theirDone = len(board.moves('2')) == 0

        if myDone or theirDone:
            if myDone and theirDone: return TIE
            if theirDone: return WIN + depth
            if myDone: return - WIN - depth

        data = makeGrid(board.width * board.height, 0)
        myqueue = [board.me]
        theirqueue = [board.them]
        myeven, myodd = 0,0
        theireven, theirodd = 0,0
        dist = 1
 
        while myqueue or theirqueue:
            myqueue = set(n for p in myqueue
                          for n in board.adjacentT(p)
                          if data[n] == 0)
            theirqueue = set(n for p in theirqueue
                             for n in board.adjacentT(p)
                             if data[n] == 0)

            both = myqueue.intersection(theirqueue)
            connected = connected or both
            myqueue.difference_update(both)
            theirqueue.difference_update(both)
            scdiff = 1 # weight / (weight + dist)
            for p in both:
                data[p] = 9
            for p in myqueue:
                if p % 2 == 0:
                    myeven += 1
                else:
                    myodd += 1
                data[p] = 1
            for p in theirqueue:
                if p % 2 == 0:
                    theireven += 1
                else:
                    theirodd += 1
                data[p] = 2
        score = getScore(myeven, myodd, board.me % 2 == 1) - \
                getScore(theireven, theirodd, board.them % 2 == 1) 
        
        if connected or isConnected(board, board.me, board.them):
            return score
        else:

            m = discore(board, board.me)
            t = discore(board, board.them)
            # db("calculating separated in alphabeta... %s  %s" 
            #   % (m - t, score))
            return m-t
        
    
class DJBot():
    def __init__(self):
        self.evals = 0
        
    def alphabeta(self, stack, evalf, node, depth, alpha, beta, startMove=None,
                  color=1, space=""):
        verbose = False
        # db("%s start alphabeta: %d %d alpha:%s beta:%s" %
        # (space, depth, color, alpha, beta))
        bestMove = []
        moves = node.moves()
        # firstMove = startMove
        term,sc = node.isTerminal(depth)
        if time.time()  > self.endTime:
            raise TimeOut()

        if term:
            return max(alpha,color * sc), bestMove
        if ((depth < 2 and node.toMove =='1')):
            bestMove = []
            sc = evalf(node,depth)
            alpha = max(alpha, color * sc)
            if verbose:
                debug("%s -- score %s  %s" ,space, sc, stack)
            
            return alpha, bestMove
        if len(moves) > 1:
            depthn = depth - 1
        else:
            depthn = depth

        if startMove and startMove in moves:
            i = moves.index(startMove)
            moves[0], moves[i] = moves[i], moves[0]
            # db("putting startmove first: %s %s" % (startMove, moves))
            # db("board:\n%s" % node)
        for move in moves:
            node.move(move)
            stack.append(move)
            tmp,junk = self.alphabeta(stack,evalf, node, depthn, -beta, -alpha,
                                            None, -1*color,
                                            space + "  ")
            stack.pop()
            node.pop()
            tmp = -tmp
            # if firstMove:
            # db("%s :: %s" % (move, tmp))
            if tmp > alpha:
                alpha = tmp
                bestMove = junk
                bestMove.append(move)
            if beta <= alpha:
                break
        # db("%s in alphabeta: d: %d c:%s alpha:%s bestMove:%s (beta:%s) %s"
        # % (space, depth, color, alpha, bestMove,beta, str(stack)))
        if verbose:
            debug("%s in alphabeta: d: %d c:%s score: %s",
               space, depth, color, alpha)
        return (alpha, bestMove)

    def which_move_alpha(self,realBoard,depth=MAXDEPTH):
        self.startTime = time.time()
        self.endTime =  0.9 + self.startTime
        evalf = Vor()

        board = Board(realBoard)
        me = board.me

        moves = realBoard.moves()
        if len(moves) == 1:
            return moves[0]
        mymoves = board.moves()
        info("Me: %s Them: %s" ,board.me, board.them)
        info("moves: %s  %s", moves, board.moves())
        # db("board:\n%s" % board)
        currentBest = board.moves()[0]

        bestMove = None
        bestScore = -WIN

        # db("adj: %s" % (board.adjacent(board.me)))
        def bestDir(me, move):
            if move ==me - 1:
                return "W"
            if move == me + 1:
                return "E"
            if move  > me:
                return "S"
            return "N"
        

        try:
            if isConnected(board, board.me, board.them):
                for d in range(2,depth+1,2):
                    bestScore,bestMoves = (
                        self.alphabeta([],evalf.f, board, d, -BIG, BIG,
                                       currentBest))
                    if len(bestMoves) > 0:
                        bestMove = bestMoves[-1]
                    else: bestMove = None
                    bestMoves.reverse()
                           
                    info("(%s) best (%d) (%s:%s) : %s %s %s",
                       currentBest, d, bestMove, bestDir(board.me, bestMove),
                        bestScore, evalf.evals, bestMoves)
                    currentBest = bestMove
                    if bestScore >= WIN: break
                    if bestScore <= -WIN: break
            else:
                bestScore = WIN
                for d in range(1, depth+1):
                    bestScore, bestMove = \
                               disconnectedsearch(board, d,
                                                  board.me, bestScore,
                                                  self.endTime)
                    
                    info("best (depth: %d) move: %s : score: %s -- disconnected",
                       d, bestMove, bestScore)
                    currentBest = bestMove
                    if bestScore + 2 < d: break
                    
        except TimeOut:
            info("TIMED OUT!")
            pass
        
        except Exception,inst        :
            logging.critical("Got exception %s %s" ,inst, inst.args)

        
        ret = moves[0]
        currentBestp = board.fromN(currentBest)
        for m in moves:
            if realBoard.rel(m) == currentBestp:
                info("found move: %s %s %s %s" ,bestMove, m,
                   bestScore,board.toN(realBoard.me()))
                ret = m
                break
        else:
            logging.critical("move search failed!")
            logging.critical("real board moves: %s" ,
               [realBoard.rel(m) for m in realBoard.moves()])
            logging.critical("board moves: %s", realBoard.moves())
            logging.critical("my moves: %s", mymoves)
            logging.critical("bestmove: %s %s" ,bestMove, board.fromN(bestMove))
               
        duration = time.time() - self.startTime
        info("time to move: %s",duration)
        evals = evalf.evals
        info("evals performed: %d",evals)
        if evals > 0:
            info("average eval time: %f", duration / evals)
        return ret

def bestDir(me, move):
    if move ==me - 1:
        return "W"
    if move == me + 1:
        return "E"
    if move  > me:
        return "S"
    return "N"



def main(argv=sys.argv):
    info("%s" % argv)
    djbot = DJBot()
    for board in TBoard.generate():
        move(djbot.which_move_alpha(board))
    info("DONE")

        

if __name__ == "__main__":
    if False:
        logging.basicConfig(level=logging.INFO,
                            stream=sys.stderr,
                            format = "db: %(levelname)-8s %(message)s")
    log.info("TEST LOGGER")    
    main()
