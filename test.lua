print(string.format("\027[7mHello from %s!\027[27m", _VERSION))

local script_dir = (debug.getinfo(1, "S").source:sub(2):match("^(.*)[/\\][^/\\]+$"))
print(string.format("script_dir = %q", script_dir))

print(dofile(script_dir .. "/Zelda64RecompSyms/mm.us.rev1.syms.lua"))
