--[[---------------------------------------------------------------------------

Sandcrawler

by luv2run

luv2runxc@gmail.com

A bot for the 2010 Google AI Challenge -- Tron.


Uses minimax to see if a sure win is possible within the next few moves,
and takes the correct move if so.

Otherwise, checks if player can reach the enemy.

If so, checks to see what move would allow the player to control the most space
relative to the opponent and takes that move.

Otherwise, tries to hug walls in an effort to survive longest.


--]]---------------------------------------------------------------------------


--[[---------------------------------------------------------------------------

Intro: constants, priority queue, built in play_tron() function.

--]]---------------------------------------------------------------------------

-- some useful constants.
local WALL, OPEN, ME, ENEMY = string.byte("# 12", 1, 4)


-- A priority queue used in dijkstra
-- store nodes in heap (array)
-- children of heap[i] are at heap[2i] and heap[2i+1]
PriorityQueue = {
    heap = {},
    values = {},
    
    push = function (self,node)
        local n = #(self.heap) + 1  -- last element plus one
        self.heap[n] = node     -- insert it in the bottom of the array
        local val = self.value(node)
        self.values[node] = val
        local par = (n - n%2)/2 -- parent
        -- rebalance tree
        while n > 1 and self.compare(val,self.values[self.heap[par]]) do
            self.heap[par],self.heap[n] = self.heap[n],self.heap[par]
            n = par
            par = (n - n%2)/2
        end
    end,
    
    pop = function (self)
        local ans = self.heap[1]
        if ans == nil then return nil end
        local size = #(self.heap)
        local new_root = self.heap[size]
        local val = self.values[new_root]
        self.heap[1] = new_root
        self.heap[size] = nil
        self.values[ans] = nil
        size = size-1
        local n = 1  -- address of guy to bubble down
        local pos1 = 2  -- address of swapper
        if size>2 and self.compare(self.values[self.heap[3]],self.values[self.heap[2]]) then
            pos1 = 3
        end
        while size >= pos1 and not self.compare(val,self.values[self.heap[pos1]]) do
            self.heap[n],self.heap[pos1] = self.heap[pos1],self.heap[n]
            n = pos1
            pos1 = n*2
            if size > pos1 and self.compare(self.values[self.heap[pos1+1]],self.values[self.heap[pos1]]) then
                pos1 = pos1 + 1
            end
        end
        
        return ans
    end,
    
    isempty = function (self)
        return (self.heap[1] == nil)
    end,
    
    size = function(self)
        return #(self.heap)
    end,
    
    value = function (node1)
        return tonumber(node1)
    end,
    
    compare = function(val1,val2)
        return (val1 < val2)
    end,
    
    new = function (self,pq)
        pq = pq or {}
        pq.heap = {}
        pq.values = {}
        setmetatable(pq,PriorityQueue)
        self.__index = self
        return pq
    end,
}



function play_tron()
   -- Build and return an iterator over successive game states.

   -- Three accessor functions are included in the game state table.
   local function is_wall(self, x, y)
      -- Indexing outside the board returns true.
      return not (self.board[y] and self.board[y][x] == OPEN)
   end

   local function my_xy(self)
      return self.player1[1], self.player1[2]
   end

   local function enemy_xy(self)
      return self.player2[1], self.player2[2]
   end

   local function read_board()
      -- Read and return a board state from standard input
      -- or return nil if input is empty.
      local first_line = io.read()
      if first_line == nil or first_line == "" then return nil end
      local width, height = first_line:match('(%d*) (%d*)')
      width = tonumber(width)
      height = tonumber(height)

      -- Build the map. Find player positions.
      local map = {}
      local player1, player2
      for i = 1, height do
         local line = io.read()
         assert(#line == width, string.format(
            'unexpected line length: %d %s', i, line))
         local tbl = {}
         for i = 1, #line do
            table.insert(tbl, line:byte(i))
         end
         table.insert(map, tbl)
         local p1, p2 = line:find('1'), line:find('2')
         if p1 then player1 = {p1, i} end
         if p2 then player2 = {p2, i} end
      end

      -- Wrap it all up in a table.
      return {board = map, player1 = player1, player2 = player2,
         width = width, height = height, is_wall = is_wall,
         my_xy = my_xy, enemy_xy = enemy_xy}
   end

   -- The function read_board is the iterator.
   return read_board
end

function make_move(move)
    io.write(move .. '\n')
    io.flush()
end


function dist_away(game_state)
    local mx,my = game_state:my_xy()
    local ex,ey = game_state:enemy_xy()
    return math.abs(mx-ex), math.abs(my-ey)
end


--[[---------------------------------------------------------------------------

Boards -- used in many of the functions.

--]]---------------------------------------------------------------------------

function isWall(board,x,y)
    return (board.map[y] and board.map[y][x] == WALL)
end

function isOpen(board,x,y)
    return (board.map[y] and board.map[y][x] == OPEN)
end

function makeBoard(game_state)
    local mx,my = game_state:my_xy()
    local ex,ey = game_state:enemy_xy()
    local w,h = game_state.width,game_state.height
    local mp = {}
    for j=1,h do
        local tbl = {}
        for i=1,w do
            tbl[i] = game_state.board[j][i]
        end
        mp[j] = tbl
    end
    return {map=mp,my_x=mx,my_y=my,e_x=ex,e_y=ey,width=w,height=h}
end

function copyBoard(board)
    local mp = {}
    local w,h = board.width,board.height
    for j=1,h do
        local tbl = {}
        for i=1,w do
            tbl[i] = board.map[j][i]
        end
        mp[j] = tbl
    end
    return {map=mp,my_x=board.my_x,my_y=board.my_y,e_x=board.e_x,e_y=board.e_y,width=w,height=h}
end



--[[---------------------------------------------------------------------------

Pathfinding: A* for seeing if we can reach the enemy,
dijkstra for checking distances to all reachable squares.

--]]---------------------------------------------------------------------------


local dx = {0,1,0,-1}
local dy = {-1,0,1,0}



-- A* from me to enemy
-- return direction, or nil if there is no route
function astar(board,currx,curry,destx,desty)
    
    -- construct map, mark walls
    local map = {}
    for j=1,board.height do
        local tbl = {}
        for i=1,board.width do
            if not isWall(board,i,j) then tbl[i] = 0
            else tbl[i] = 1 end
        end
        map[j] = tbl
    end
    
    -- make paths diagonal
    --local correction = 1 + 1.0/(math.abs(currx-destx)+math.abs(curry-desty))
    
    local h_val = function(x,y)
        return math.sqrt((destx-x)*(destx-x)+(desty-y)*(desty-y))    -- * correction
    end
    
    local pq = PriorityQueue:new{value = function(node) return node.f end,compare = function(val1,val2) return (val1 < val2) end}
    
    -- list containing the first node, mark it
    local h2 = h_val(currx,curry)
    pq:push{x=currx,y=curry,distance=0,h=h2,f=h2,parent=nil,dir=nil}
    map[curry][currx] = 0
    
    local done=nil
    while not pq:isempty() and not done do
        curr = pq:pop()
        if map[curr.y][curr.x] == 0 then
            map[curr.y][curr.x] = 1
            for mydir=1,4 do
                local x,y = curr.x+dx[mydir],curr.y+dy[mydir]
                if x==destx and y==desty then done={x=x,y=y,parent=curr,dir=mydir} break end 
                if map[y][x] == 0 then
                    local g = curr.distance+1
                    local h = h_val(x,y)
                    local node = {x=x,y=y,distance=g,h=h,f=g+h,parent=curr,dir=mydir}
                    pq:push(node)
                end
            end
        end
    end
    
    if done then
        local d = nil
        while done.parent do
            d = done.dir
            done = done.parent
        end
        return d
    else
        return nil
    end
end



-- return a map of distances to every accessible node
-- if we can reach the node from the starting point, map[y][x] = distance to node
-- if the node is a wall, map[y][x] = -2
-- if the node is open but unreachable, map[y][x] = -1
function dijkstra(board,currx,curry)
    
    -- construct map, mark walls
    local map = {}
    for j=1,board.height do
        local tbl = {}
        for i=1,board.width do
            if board.map[j][i] ~= OPEN then tbl[i] = -2
            else tbl[i] = -1 end
        end
        map[j] = tbl
    end
    
    -- list containing the first node, mark it
    local pq = PriorityQueue:new{value = function(node) return node.distance end, compare = function(val1,val2) return (val1 < val2) end}
    pq:push{x=currx,y=curry,distance=0}
    map[curry][currx] = -1
    
    while not pq:isempty() do
        curr = pq:pop()
        if map[curr.y][curr.x] == -1 then   -- open node
            map[curr.y][curr.x] = curr.distance
            for mydir=1,4 do
                local x,y = curr.x+dx[mydir],curr.y+dy[mydir]
                if map[y][x] == -1 then
                    local node = {x=x,y=y,distance=curr.distance+1}
                    pq:push(node)
                end
            end
        end
    end
    
    return map
end

-- Exactly the same as dijkstra, but also return a list of reachable squares
function dijkstra2(board,currx,curry)
    
    -- construct map, mark walls
    local map = {}
    for j=1,board.height do
        local tbl = {}
        for i=1,board.width do
            if board.map[j][i] ~= OPEN then tbl[i] = -2
            else tbl[i] = -1 end
        end
        map[j] = tbl
    end
    
    local list = {}
    
    -- list containing the first node, mark it
    local pq = PriorityQueue:new{value = function(node) return node.distance end, compare = function(val1,val2) return (val1 < val2) end}
    pq:push{x=currx,y=curry,distance=0}
    map[curry][currx] = -1
    
    while not pq:isempty() do
        curr = pq:pop()
        if map[curr.y][curr.x] == -1 then   -- open node
            map[curr.y][curr.x] = curr.distance
            table.insert(list,{curr.x,curr.y})
            for mydir=1,4 do
                local x,y = curr.x+dx[mydir],curr.y+dy[mydir]
                if map[y][x] == -1 then
                    local node = {x=x,y=y,distance=curr.distance+1}
                    pq:push(node)
                end
            end
        end
    end
    
    return map,list
end


--[[---------------------------------------------------------------------------

Wall Hugger -- Survival mode

--]]---------------------------------------------------------------------------

local DEADEND = 1
local TUNNEL = 2
local CORNER = 3
local TRI = 4
local ALLOPEN = 5

-- return which of the above types it is
function wallType(board,sx,sy)
    local num=0
    local walls = {}
    for d=1,4 do
        if not isOpen(board,sx+dx[d],sy+dy[d]) then
            table.insert(walls,d)
            num = num+1
        end
    end
    if num==0 then return ALLOPEN end
    if num==1 then return TRI end
    if num==2 then
        -- if walls has 2 and 4 or 1 and 3, its a tunnel
        num = math.abs(walls[1] - walls[2])
        if num == 2 then return TUNNEL end
        return CORNER
    end
    return DEADEND
end

-- how many walls does the square touch?
function wallTouches(board,x,y)
    local num=0
    for d=1,4 do
        if not isOpen(board,x+dx[d],y+dy[d]) then
            num = num+1
        end
    end
    return num
end


-- utility function for WallHugger checking if squares around me can all
-- reach each other
function checkReaches(board)
    local canreach=true
    local wallcounts = {-1,-1,-1,-1}
    local opendirs = {}
    for mydir=1,4 do
        local x,y = board.my_x+dx[mydir],board.my_y+dy[mydir]
        if isOpen(board,x,y) then table.insert(opendirs,mydir) end
    end
    
    -- make player, enemy into walls
    board.map[board.my_y][board.my_x] = WALL
    board.map[board.e_y][board.e_x] = WALL
    
    local i=1
    while opendirs[i] do
        local mydir = opendirs[i]
        
        local sx,sy = board.my_x+dx[mydir],board.my_y+dy[mydir]
        
        if canreach then
            local j=i+1
            while opendirs[j] do
                local odir = opendirs[j]
                
                
                local destx,desty = board.my_x+dx[odir],board.my_y+dy[odir]
                local move = astar(board,sx,sy,destx,desty)
                if not move then canreach = false break end
                
                j = j+1
            end
        end
        
        -- count how many squares around are walls
        wallcounts[mydir] = wallTouches(board,sx,sy)
        
        i = i+1
    end
    
    board.map[board.my_y][board.my_x] = ME
    board.map[board.e_y][board.e_x] = ENEMY
    
    return wallcounts,canreach,opendirs
end


-- values of a deadend, tunnel, corner, tri, and allopen square
local hugwts = {.01,1,2,2.5,3}

-- Tries to hug walls as much as possible without splitting into two spaces,
-- if it must, chooses the more open one.
function WallHugger(board)
    
    local direction = 1
    
    -- can the squares around me all reach each other?
    local wallcounts,canreach,opendirs = checkReaches(board)
    
    -- if so
    local mostopendir = 1
    local numopen = 0
    if canreach then
        
        local foundone = false
        
        local nochoice = false
        
        while true do
            
            nochoice = true
            -- pick one that touches the most walls
            local bestcount=0
            local bestdir=1
            for d=1,4 do
                if wallcounts[d] > bestcount then
                    bestcount = wallcounts[d]
                    bestdir = d
                    nochoice = false
                end
            end
            
            if nochoice then break end
            
            -- make move, can all sides reach each other?
            local mx,my = board.my_x,board.my_y
            board.my_x = mx + dx[bestdir]
            board.my_y = my + dy[bestdir]
            board.map[my][mx] = WALL
            board.map[board.my_y][board.my_x] = ME
            
            -- can all squares still reach each other?
            local counts,nowreach,dirs = checkReaches(board)
            
            board.map[my][mx] = ME
            board.map[board.my_y][board.my_x] = OPEN
            board.my_x = mx
            board.my_y = my
            
            -- if so, good
            if nowreach == true then
                foundone = true
                direction = bestdir
                break
            end
            
            wallcounts[bestdir] = -1
        end
        
        if foundone then
            return direction
        end
        
    end
    
    -- pick direction with most open space
    local i=1
    local spaceamt = {-1,-1,-1,-1}
    while opendirs[i] do
        
        local mydir = opendirs[i]
        
        local mx,my = board.my_x,board.my_y
        board.my_x = mx + dx[mydir]
        board.my_y = my + dy[mydir]
        board.map[my][mx] = WALL
        board.map[board.my_y][board.my_x] = ME
        
        local mmap,mlist = dijkstra2(board,board.my_x,board.my_y)
        
        -- weight the nodes
        local score = 0
        for i,node in ipairs(mlist) do
            score = score + hugwts[wallType(board,node[1],node[2])]
        end
        
        spaceamt[mydir] = score
        
        board.map[my][mx] = ME
        board.map[board.my_y][board.my_x] = OPEN
        board.my_x = mx
        board.my_y = my
        
        i = i+1
    end
    
    local best=0
    for d=1,4 do
        if spaceamt[d] > best then
            direction = d
            best = spaceamt[d]
        end
    end
    
    return direction
    
end






--[[---------------------------------------------------------------------------

Minimax
Looks ahead as much as possible to see if there's a sure win for us in any
direction.
If so, returns the direction to move.
Limits the amount of looking with a count variable.

--]]---------------------------------------------------------------------------





local MAXDEPTH = 9
local MAXCOUNT = 500
local WIN = 1000
local LOSS = -1000
local dx = {0,1,0,-1}
local dy = {-1,0,1,0}


-- if there's a sure win (or sure loss?), return a move to
-- take it (or prevent it?)
function count_minimax(board,count)
    
    local my_board = copyBoard(board)   -- copy board, change to new state
    
    local num_losses = 0
    local myopens = {}
    for mydir=1,4 do
        if isWall(board,board.my_x+dx[mydir],board.my_y+dy[mydir]) then
            num_losses = num_losses + 1
        else
            table.insert(myopens,mydir)
        end
    end
    
    local num_wins = 0
    local eopens = {}
    for mydir=1,4 do
        if isWall(board,board.e_x+dx[mydir],board.e_y+dy[mydir]) then
            num_wins = num_wins + 1
        else
            table.insert(eopens,mydir)
        end
    end
    
    if num_losses == 4 then
        return nil
    elseif num_wins == 4 then
        return WIN
    end
    
    if count < 1 then
        return nil
    end
    
    local best = -1000
    for a,mydir in ipairs(myopens) do
        
        local num_wins = 0
        
        for b,mydir in ipairs(eopens) do
            local size = (#myopens * #eopens)
            
            my_board.my_y = board.my_y+dy[mydir]
            my_board.my_x = board.my_x+dx[mydir]
            my_board.e_y = board.e_y+dy[mydir]
            my_board.e_x = board.e_x+dx[mydir]
            
            if my_board.my_y == my_board.e_y or my_board.e_x == my_board.e_x then
                
            else
            
                -- move the player
                my_board.map[my_board.my_y][my_board.my_x] = WALL
                my_board.map[my_board.my_y][my_board.my_x] = ME
                my_board.map[my_board.e_y][my_board.e_x] = WALL
                my_board.map[my_board.e_y][my_board.e_x] = ENEMY
                
                local temp = count_minimax(my_board,(count-size)/(size))
                
                -- if we win, then return a win!
                if temp==WIN then num_wins = num_wins+1 end
                
                -- put the player back where he was
                my_board.map[my_board.my_y][my_board.my_x] = OPEN
                my_board.map[my_board.my_y][my_board.my_x] = ME
                my_board.map[my_board.e_y][my_board.e_x] = OPEN
                my_board.map[my_board.e_y][my_board.e_x] = ENEMY
            end
            my_board.my_x = board.my_x
            my_board.my_y = board.my_y
            my_board.e_x = board.e_x
            my_board.e_y = board.e_y
        end
        
        if num_wins == (#eopens) then 
            return mydir
        end
        
    end
    
    return nil
    
end


-- minimax bot
function miniBot(game_state)
    
    return count_minimax(makeBoard(game_state),MAXCOUNT)
    
end


--[[---------------------------------------------------------------------------

CheckFlood: checks who controls more space

--]]---------------------------------------------------------------------------


-- indexed by number of connected open squares
local floodwts = {.01,1,2,2.5,3}

function blankScore(board,x,y)
    return floodwts[wallType(board,x,y)]
end


-- given a board, check who controls the most space
-- by dijkstra-ing distance to each square for both players,
-- and comparing the distances
-- squares are given different weights; it's more valuable
-- to control more open squares
function checkFloodWt(board)
    -- get shortest path to all nodes we can reach
    local mmap = dijkstra(board,board.my_x,board.my_y)
    
    -- enemy
    local emap = dijkstra(board,board.e_x,board.e_y)
    
    local diff = 0
    for i=1,board.width do
        for j=1,board.height do
            if board.map[j][i] == OPEN then
                if mmap[j][i] < 0 then
                    if emap[j][i] > 0 then
                        local num = blankScore(board,i,j)
                        diff = diff-num
                    end
                elseif emap[j][i] < 0 then
                    local num = blankScore(board,i,j)
                    diff = diff+num
                else
                    local d = mmap[j][i] - emap[j][i]   -- distance - distance
                    local num = blankScore(board,i,j)
                    if d < 0 then diff = diff + num       -- we're closer
                    elseif d > 0 then diff = diff - num end   -- she's closer
                end
            end
        end
    end
    
    return diff
end


--[[---------------------------------------------------------------------------

Simple and Complex Bots -- the main bots for if we can reach the enemy.
SimpleBot compares the amount of space we control to the enemy for each
of our possible moves (without moving the enemy) and picks the best,
adjusting for the possiblity of draws.
ComplexBot3 also moves the enemy.
ComplexBot4 looks ahead one turn.

--]]---------------------------------------------------------------------------


function rankTypes(type)
    if type==CORNER then return 1
    elseif type == TRI then return 2
    elseif type == OPEN then return 3
    elseif type == TUNNEL then return 4
    else return 5 end
end


-- Move us in each possible direction, checking which would control the most
-- space compared to the enemy.
function simpleBot(game_state)
    local best=-1000
    local bestdir=1
    local board = makeBoard(game_state)
    local dists = {-1000,-1000,-1000,-1000}
    local draw = {}
    for mydir=1,4 do
        
        local mx,my = board.my_x,board.my_y
        board.my_x = mx + dx[mydir]
        board.my_y = my + dy[mydir]
        if board.map[board.my_y][board.my_x] == OPEN then
            
            board.map[my][mx] = WALL
            board.map[board.my_y][board.my_x] = ME
            
            local dist = checkFloodWt(board)
            
            dists[mydir] = dist
            
--            io.write(mydir .. " : score : " .. dist .. "\n")
            
            if dist > best then
                best = dist
                bestdir = mydir
            end
            
            for edir=1,4 do
                local ex,ey = board.e_x+dx[edir],board.e_y+dy[edir]
                if ex == board.my_x and ey == board.my_y then
                    table.insert(draw,mydir)
                end
            end
            
            board.map[my][mx] = ME
            board.map[board.my_y][board.my_x] = OPEN
        end
        board.my_x = mx
        board.my_y = my
    end
    
    for i,dir in ipairs(draw) do
        if dir == bestdir then
            local sdir = 1
            local sec = -1000
            for d=1,4 do
                if d ~= bestdir and dists[d] > sec then
                    sdir = d
                    sec = dists[d]
                end
            end
            if sec > 5 then
                bestdir = sdir
            end
        end
    end
    
    return bestdir
end



-- Moves us in each possible direction.
-- For each of those: move the enemy in each possible direction.
-- Pick his best move (our worst score), and assign that score to our direction.
-- Pick our direction with the best worst score.
function complexBot3(game_state)
    local best=-1000
    local bestdir=1
    local board = makeBoard(game_state)
    local dists = {-1000,-1000,-1000,-1000}
    local draw = {}
    for mydir=1,4 do
        
        local mx,my = board.my_x,board.my_y
        board.my_x = mx + dx[mydir]
        board.my_y = my + dy[mydir]
        if board.map[board.my_y][board.my_x] == OPEN then
            
            board.map[my][mx] = WALL
            board.map[board.my_y][board.my_x] = ME
            
            local scores = {1000,1000,1000,1000}
            for edir=1,4 do
                
                local ex,ey = board.e_x,board.e_y
                board.e_x = ex + dx[edir]
                board.e_y = ey + dy[edir]
                
                if board.e_x == board.my_x and board.e_y == board.my_y then
                    table.insert(draw,mydir)
                elseif board.map[board.e_y][board.e_x] == OPEN then
                    board.map[ey][ex] = WALL
                    board.map[board.e_y][board.e_x] = ENEMY
                    
                    local score = checkFloodWt(board)
                    
--                    io.write("I go " .. mydir .. " and he goes " .. edir .. ": " .. score .. "\n")
                    scores[edir] = score
                    
                    board.map[board.e_y][board.e_x] = OPEN
                    board.map[ey][ex] = ENEMY
                end
                
                board.e_x = ex
                board.e_y = ey
                
            end
            
            local worst = 1000
            local total = 0
            for i=1,4 do
                total = total + scores[i]
                if scores[i] < worst then worst = scores[i] end
            end
            local dist = worst  -- use worst case
            
            dists[mydir] = dist
            
--            io.write(mydir .. " : score : " .. dist .. "\n")
            
            if dist > best then
                best = dist
                bestdir = mydir
            end
            
            board.map[my][mx] = ME
            board.map[board.my_y][board.my_x] = OPEN
        end
        board.my_x = mx
        board.my_y = my
    end
    
    for i,dir in ipairs(draw) do
        if dir == bestdir then
            local sdir = 1
            local sec = -1000
            for d=1,4 do
                if d ~= bestdir and dists[d] > sec then
                    sdir = d
                    sec = dists[d]
                end
            end
            -- if we have another choice scoring over 5, avoid the draw
            if sec > 5 then
                bestdir = sdir
            end
        end
    end
    
    return bestdir
end




-- this is for complex bot 4, it's very redundant to complexbot 3
-- just looks ahead one
function RecurBot(board,depth)
    
    local best=-1000
    local bestdir=1
    local dists = {-1000,-1000,-1000,-1000}
    local draw = {}
    for mydir=1,4 do
        
        local mx,my = board.my_x,board.my_y
        board.my_x = mx + dx[mydir]
        board.my_y = my + dy[mydir]
        if board.map[board.my_y][board.my_x] == OPEN then
            
            board.map[my][mx] = WALL
            board.map[board.my_y][board.my_x] = ME
            
            local scores = {1000,1000,1000,1000}
            for edir=1,4 do
                
                local ex,ey = board.e_x,board.e_y
                board.e_x = ex + dx[edir]
                board.e_y = ey + dy[edir]
                
                if board.e_x == board.my_x and board.e_y == board.my_y then
                    table.insert(draw,mydir)
                elseif board.map[board.e_y][board.e_x] == OPEN then
                    board.map[ey][ex] = WALL
                    board.map[board.e_y][board.e_x] = ENEMY
                    
                    local score
                    if depth == 0 then score = checkFloodWt(board)
                    else depth = recurBot(board,depth-1)
                    end
                    
--                    io.write("I go " .. mydir .. " and he goes " .. edir .. ": " .. score .. "\n")
                    scores[edir] = score
                    
                    board.map[board.e_y][board.e_x] = OPEN
                    board.map[ey][ex] = ENEMY
                end
                
                board.e_x = ex
                board.e_y = ey
                
            end
            
            local worst = 1000
            local total = 0
            for i=1,4 do
                total = total + scores[i]
                if scores[i] < worst then worst = scores[i] end
            end
            local dist = worst  -- use worst case
            
            dists[mydir] = dist
            
--            io.write(mydir .. " : score : " .. dist .. "\n")
            
            if dist > best then
                best = dist
                bestdir = mydir
            end
            
            board.map[my][mx] = ME
            board.map[board.my_y][board.my_x] = OPEN
        end
        board.my_x = mx
        board.my_y = my
    end
    
     for i,dir in ipairs(draw) do
        if dir == bestdir then
            best = best*.8
        end
    end
    
    return best
    
end



-- Very much redundant code, calls recurBot with no additional depth
-- so it only looks ahead once (too slow otherwise).
function complexBot4(game_state)
    local best=-1000
    local bestdir=1
    local board = makeBoard(game_state)
    local dists = {-1000,-1000,-1000,-1000}
    local draw = {}
    for mydir=1,4 do
        
        local mx,my = board.my_x,board.my_y
        board.my_x = mx + dx[mydir]
        board.my_y = my + dy[mydir]
        if board.map[board.my_y][board.my_x] == OPEN then
            
            board.map[my][mx] = WALL
            board.map[board.my_y][board.my_x] = ME
            
            local scores = {1000,1000,1000,1000}
            for edir=1,4 do
                
                local ex,ey = board.e_x,board.e_y
                board.e_x = ex + dx[edir]
                board.e_y = ey + dy[edir]
                
                if board.e_x == board.my_x and board.e_y == board.my_y then
                    table.insert(draw,mydir)
                elseif board.map[board.e_y][board.e_x] == OPEN then
                    board.map[ey][ex] = WALL
                    board.map[board.e_y][board.e_x] = ENEMY
                    
                    local score = RecurBot(board,0)
                    
--                    io.write("I go " .. mydir .. " and he goes " .. edir .. ": " .. score .. "\n")
                    scores[edir] = score
                    
                    board.map[board.e_y][board.e_x] = OPEN
                    board.map[ey][ex] = ENEMY
                end
                
                board.e_x = ex
                board.e_y = ey
                
            end
            
            local worst = 1000
            local total = 0
            for i=1,4 do
                total = total + scores[i]
                if scores[i] < worst then worst = scores[i] end
            end
            local dist = worst  -- use worst case
            
            dists[mydir] = dist
            
--            io.write(mydir .. " : score : " .. dist .. "\n")
            
            if dist > best then
                best = dist
                bestdir = mydir
            end
            
            board.map[my][mx] = ME
            board.map[board.my_y][board.my_x] = OPEN
        end
        board.my_x = mx
        board.my_y = my
    end
    
    for i,dir in ipairs(draw) do
        if dir == bestdir then
            local sdir = 1
            local sec = -1000
            for d=1,4 do
                if d ~= bestdir and dists[d] > sec then
                    sdir = d
                    sec = dists[d]
                end
            end
            if sec > 5 then
                bestdir = sdir
            end
        end
    end
    
    return bestdir
end



--[[---------------------------------------------------------------------------

Main AI controller and game loop.

--]]---------------------------------------------------------------------------




-- run the bot!
-- Uses minimax to see if there's a sure win -- if so, take it
-- Then uses A* to see if there's a path to the enemy.
-- If so, depending on map size uses a bot to control the most space.
-- If not, uses WallHugger to survive.
function runBot(game_state)
    
    -- if we can win for sure, do it!
    local move = miniBot(game_state)
    if move then return move end
    
    -- see if we can reach opponent at all
    local board = makeBoard(game_state)
    move = astar(board,board.my_x,board.my_y,board.e_x,board.e_y)
        
    -- if so:
    if move then
        local size = game_state.width * game_state.height
        
        if size < 300 then
            move = complexBot4(game_state)
        elseif size <= 650 then
            move = complexBot3(game_state)
        else
            move = simpleBot(game_state)
        end
    else
        -- if not, just spacecrawl
        move = WallHugger(makeBoard(game_state))
    end
    
    
    return move
end




-- This loop runs the bot for as long as new boards come in.

-- The game state table contains the following fields:
--   board : table of tables containing either WALL, OPEN, ME, or ENEMY
--   player1, player2: x y pairs of positions, player2 is the enemy bot
--   width, height: the width and height of the board
--   is_wall(x, y): queries whether there is a wall at position
--   my_xy(): returns two values, the x and y coordinates of your bot
--   enemy_xy(): return two values, the x and y coordinates of the enemy bot

for game_state in play_tron() do
   local move = runBot(game_state)
   make_move(move)
end