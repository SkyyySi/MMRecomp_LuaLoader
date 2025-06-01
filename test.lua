return select(2, assert(xpcall(function(...)
	print(string.format("\027[7mHello from %s!\027[27m", _VERSION))

	print(string.format("Recomp: %s = %s -> %s", type(Recomp), tostring(Recomp), tostring(getmetatable(Recomp))))
	assert(type(Recomp) == "table")
	--[[ for k, v in pairs(Recomp) do
		print(string.format("Recomp.%s = %s -> %s", tostring(k), tostring(v), tostring(getmetatable(v))))
	end ]]
	local str = ""
	for i = 1, 31 do
		local index = (0x801f9ad8 + i) & 0x7FFFFFFF
		local byte = Recomp.rdram[index]
		local char = string.char(byte)
		str = str .. char
		-- print(string.format("Recomp.rdram[0x%08X] = 0x%02X -> %q", index, byte, char))
	end
	print(string.format("  -> %q", str))

	do return end

	local script_dir = (debug.getinfo(1, "S").source:sub(2):match("^(.*)[/\\][^/\\]+$"))
	print(string.format("script_dir = %q", script_dir))

	print(dofile(script_dir .. "/Zelda64RecompSyms/mm.us.rev1.syms.lua"))
end, function(err, ...)
	print(debug.traceback(err, 2))
	return err, ...
end, ...)))
