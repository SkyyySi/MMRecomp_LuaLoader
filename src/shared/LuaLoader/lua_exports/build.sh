#!/usr/bin/env bash
cd "$(dirname -- "${BASH_SOURCE[0]}")" || exit 1

eval "$(luarocks path --lua-version '5.4')"

function run() {
	local -a cflags=() ldflags=()

	# shellcheck disable=SC2207
	cflags=(
		-Wall
		-Wpedantic
		-Werror=implicit{,-{fallthrough,function-declaration,int}}
		-std=gnu23
		-shared
		-fPIC
		$(pkg-config --cflags lua-5.4)
	)

	# shellcheck disable=SC2207
	ldflags=(
		$(pkg-config --libs lua-5.4)
	)

	gcc "${cflags[@]}" -o rdram.so rdram.c "${ldflags[@]}" &&

	lua5.4 -e 'do
		local pretty = require("pretty")
		local print = pretty.print
		local rdram = require("rdram")
		print(rdram)
		print(debug.getmetatable(rdram))
	end'
	echo "\$? = $?"
}

function main() {
	local file=''
	file=$(inotifywait --quiet --format '%w%f' --event 'modify' --include '\.(c|h)$' -- './')

	clear
	echo "File: $file"
	run
}

clear
run
while true; do
	main
done
