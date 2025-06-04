local require = require
local select  = select
local xpcall  = xpcall
local debug   = debug
local _G      = _G

local rawgetmetatable = getmetatable
if type(debug) == "table" then
	local debug_getmetatable = debug.getmetatable
	if type(debug_getmetatable) == "function" then
		rawgetmetatable = debug_getmetatable
	end
end

local print = select(2, xpcall(function()
	return require("pretty").print
end, function()
	return _G.print
end))

local rdram = require("rdram")

io.write("rdram = ")
print(rdram)

local rdram_metatable = rawgetmetatable(rdram)
io.write("rawgetmetatable(rdram) = ")
print(rdram_metatable)
