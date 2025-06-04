local require      = require
local select       = select
local xpcall       = xpcall
local print        = print
local setmetatable = setmetatable
local assert       = assert
local load         = load
local _G           = _G
local io           = io
local debug        = debug
local string       = string

--------------------------------------------------------------------------------

local pprint = select(2, xpcall(function()
	return require("pretty").print
end, function()
	return _G.print
end))

--------------------------------------------------------------------------------

---@class LuaLoader.RDRAM : userdata
---@field get_length             fun(self: self)
---@field get_capacity           fun(self: self)
---@field get_data_as_string     fun(self: self)
---@field get_raw_data_as_string fun(self: self)
---@field get_data_as_table      fun(self: self)
---@field get_raw_data_as_table  fun(self: self)
---@field read_value_s8          fun(self: self, index: integer): integer
---@field read_value_s16         fun(self: self, index: integer): integer
---@field read_value_s32         fun(self: self, index: integer): integer
---@field read_value_s64         fun(self: self, index: integer): integer
---@field read_value_u8          fun(self: self, index: integer): integer
---@field read_value_u16         fun(self: self, index: integer): integer
---@field read_value_u32         fun(self: self, index: integer): integer
---@field read_value_u64         fun(self: self, index: integer): integer
---@field read_value_f32         fun(self: self, index: integer): integer
---@field read_value_f64         fun(self: self, index: integer): integer
---@field next_pair_s8           fun(self: self, index: integer): (integer, integer)?
---@field next_pair_s16          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_s32          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_s64          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_u8           fun(self: self, index: integer): (integer, integer)?
---@field next_pair_u16          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_u32          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_u64          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_f32          fun(self: self, index: integer): (integer, integer)?
---@field next_pair_f64          fun(self: self, index: integer): (integer, integer)?
local rdram = require("rdram")

--------------------------------------------------------------------------------

---@generic R
---@param f fun(): ...: R?
---@return R ...
local function try(f)
	return select(2, xpcall(f, function(err)
		print("\027[0;1;41m ERROR \027[0m " .. debug.traceback(err))
	end))
end

---@param expression string
---@param env?       { [string]: any }
local function test(expression, env)
	if env == nil then
		env = {}
	end
	if env.rdram == nil then
		env.rdram = rdram
	end

	setmetatable(env, {
		__index = _G,
	})

	return try(function()
		local f = assert(load(
			"return " .. expression,
			string.format("Test %q", expression),
			"t",
			env
		))

		io.write("\027[0;1;35m>>>\027[0m ", expression, " \027[0;1;35m->\027[0m ")
		local results = table.pack(f())
		io.write("\027[0;1;42m OK \027[0m [", results.n or #results, "] ")
		pprint(table.unpack(results, 1, results.n))
	end)
end

--------------------------------------------------------------------------------

test("debug.getmetatable(rdram)")
test("rdram:get_capacity()")
test("rdram:get_length()")
test("#rdram")

test("pairs(rdram)")
for k, v in pairs(rdram) do
	pprint(k, v)
	if k == "get_data_as_string" then
		break
	end
end

local length = #rdram
try(function()
	local rdram_dump_file <close> = assert(io.open("./rdram-dump.bin"))
	local rdram_dump = assert(rdram_dump_file:read("*a"))
	print(string.format("rdram_dump = %q", rdram_dump:sub(length - 80, length)))
end)
do
	local buffer = {}
	for i = length - 80, length do
		buffer[#buffer+1] = string.char(rdram[i])
	end
	print(string.format("buffer = %q", table.concat(buffer)))
end
