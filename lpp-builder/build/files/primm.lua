--------------------------------------------------
--  Primm Algorithm implementation for lpp-3ds  --
--------------------------------------------------

map = {}
start = {}
seed = 0
wmap = 0
hmap = 0

function PrimmAlgorithm(r, c, cseed)
	
	-- Resetting map
	map = {}
	
	-- Selecting a seed for random purposes
	seed = cseed
	if cseed == nil then
		h,m,s = System.getTime() 
		dv,d,m,y = System.getDate()
		seed = s + 60*s + h*3600 + d*24*3600
	end
	math.randomseed(seed)
	
	-- Initializing labyrinth
	grid = {}
	frontiers = {}
	map_width = r
	map_height = c
	
	-- AddFrontiers function used by algorithm
	function AddFrontiers(xval, yval)
		if xval + 2 < map_width - 1 and grid[yval][xval+2].val == W and grid[yval][xval+2].fr then
			table.insert(frontiers, {x = xval + 2, y = yval})
			grid[yval][xval+2].fr = false
		end
		if xval - 2 > -1 and grid[yval][xval-2].val == W and grid[yval][xval-2].fr then
			table.insert(frontiers, {x = xval - 2, y = yval})
			grid[yval][xval-2].fr = false
		end
		if yval + 2 < map_height-1 and grid[yval+2][xval].val == W and grid[yval+2][xval].fr then
			table.insert(frontiers, {x = xval, y = yval + 2})
			grid[yval+2][xval].fr = false
		end
		if yval - 2 > -1 and grid[yval-2][xval].val == W and grid[yval-2][xval].fr then
			table.insert(frontiers, {x = xval, y = yval - 2})
			grid[yval-2][xval].fr = false
		end
	end
	
	-- FreeFrontiers function used by algorithm
	function FreeFrontiers(xval, yval)
		res = {}
		if xval + 2 < map_width-1 and grid[yval][xval+2].val == 0 then
			table.insert(res, {x = xval + 2, y = yval})
		end
		if xval - 2 > -1 and grid[yval][xval-2].val == 0 then
			table.insert(res, {x = xval - 2, y = yval})
		end
		if yval + 2 < map_height-1 and grid[yval+2][xval].val == 0 then
			table.insert(res, {x = xval, y = yval + 2})
		end
		if yval - 2 > -1 and grid[yval-2][xval].val == 0 then
			table.insert(res, {x = xval, y = yval - 2})
		end
		return res
	end
	
	-- Connect function used by algorithm
	function Connect(first, second)
		xdiff = first.x - second.x
		ydiff = first.y - second.y
		if xdiff ~= 0 then
			grid[first.y][first.x].val = 0
			grid[second.y][second.x].val = 0
			if xdiff > 0 then
				grid[first.y][first.x - 1].val = 0
			else
				grid[first.y][first.x + 1].val = 0
			end
		else
			grid[first.y][first.x].val = 0
			grid[second.y][second.x].val = 0
			if ydiff > 0 then
				grid[first.y - 1][first.x].val = 0
			else
				grid[first.y + 1][first.x].val = 0
			end
		end
	end
	
	-- Generating basic grid
	i = 0
	while i < c do
		j = 0
		grid[i] = {}
		while j < r do
			grid[i][j] = {}
			grid[i][j].val = W
			grid[i][j].fr = true
			j = j + 1
		end
		i = i + 1
	end
	
	-- Selecting starting point
	cell = { x = math.random(0,r-1), y = math.random(0,c-1) }
	start = cell	
	grid[cell.y][cell.x].val = 0
	AddFrontiers(cell.x, cell.y)
	
	while #frontiers > 0 do
		s = math.random(1,#frontiers)
		avail_passages = FreeFrontiers(frontiers[s].x,frontiers[s].y)
		if #avail_passages > 0 then
			new_passage = math.random(1,#avail_passages)
			Connect(frontiers[s],avail_passages[new_passage])
			AddFrontiers(frontiers[s].x,frontiers[s].y)
			final = {x = frontiers[s].x, y = frontiers[s].y}
		end
		table.remove(frontiers, s)
	end
	
	grid[final.y][final.x].val = 1
	return grid
	
end

function PrimmBuild(map_width, map_height, cseed)
	raw_map = PrimmAlgorithm(map_width, map_height, cseed)
	i = 0
	while i < map_height do
		table.insert(map, W)
		if i == 0 then
			while i <= map_height + 1 do
				table.insert(map, W)
				i = i + 1
			end
			i = 0
		end
		j = 0
		while j < map_width do
			table.insert(map, raw_map[i][j].val)
			j = j + 1
		end
		table.insert(map,W)
		i = i + 1
		if i == map_height then
			i = 0
			while i <= map_height+ 1 do
				table.insert(map, W)
				i = i + 1
			end
		end
	end
	map_width = map_width + 2
	map_height = map_height + 2
	RayCast3D.loadMap(map, map_width, map_height, tile_size, wall_height)
	RayCast3D.spawnPlayer((start.x + 1) * 64 + 32, (start.y + 1) * 64 + 32, 90)
	wmap = map_width
	hmap = map_height
end