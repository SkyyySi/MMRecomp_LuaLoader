#!/usr/bin/env bash
set -euCo pipefail
declare -g script_path=''
script_path="$(realpath --physical -- "${BASH_SOURCE[0]:-$0}")"
cd "$(dirname -- "$script_path")" || exit 1
declare -g -a script_args=( "$@" )

declare -g cc="${CC:-gcc}"
declare -g lua_version="${LUA_VERSION:-5.4}"
declare -g -a extra_cflags=()
declare -g -a extra_ldflags=()

declare -g cc_type=''
case "$("$cc" -v 2>&1)" in
	(*clang*) cc_type='clang';;
	(*gcc*)   cc_type='gcc';;
	(*)
		printf '>>> ERROR: The C-compiler "%q" does not appear to be either Clang or GCC!\n' "$cc" >&2
		exit 1
	;;
esac
declare -g cc_version=''
cc_version="$("$cc" -dumpversion)"
declare -g cc_major_version="${cc_version//.*}"

if [[ -v CFLAGS ]]; then
	IFS=$' \n\r\t\v' read -r -s -a extra_cflags <<< "$CFLAGS"
fi

if [[ -v LDFLAGS ]]; then
	IFS=$' \n\r\t\v' read -r -s -a extra_ldflags <<< "$LDFLAGS"
fi

function try() {
	local -a argv=( "$@" )

	local -i exit_code=0
	"${argv[@]}" || exit_code="$?"

	if (( exit_code == 0 )); then
		return 0
	fi

	printf \
		'\n>>> Command "%q" exited with non-zero exit code \e[1;31m%d\e[22;39m!\n' \
		"${argv[0]}" \
		"$exit_code" 1>&2

	return "$exit_code"
}

function build() {
	local -a cflags=() ldflags=()

	IFS=' ' read -r -s -a  cflags <<< "$(pkg-config --cflags -- "lua-$lua_version")"
	IFS=' ' read -r -s -a ldflags <<< "$(pkg-config --libs   -- "lua-$lua_version")"

	local cstd='gnu23'
	if  [[ "$cc_type" == 'gcc'   && "$cc_major_version" -lt 12 ]] ||
		[[ "$cc_type" == 'clang' && "$cc_major_version" -lt 16 ]]; then
		cstd='gnu2x'
	fi
	#typeset -p cstd cc_type cc_major_version 1>&2
	#ls --color=auto -dl -- "$(which "$cc")" 1>&2

	local -a cc_argv=(
		"$cc"
		#-Wall
		-Wpedantic
		-Werror=implicit
		-Werror=implicit-{fallthrough,function-declaration,int}
		-std="$cstd"
		-shared
		-fPIC
		-D'_GNU_SOURCE'
		"${cflags[@]}"
		"${extra_cflags[@]}"
		-o './rdram.so'
		'./rdram.c'
		"${ldflags[@]}"
		"${extra_ldflags[@]}"
	)
	#typeset -p cc_argv 1>&2

	try "${cc_argv[@]}"
}

function run() {
	try lua5.4 -- './test.lua'
}

function rebuild_and_run() {
	clear

	if (( $# == 0 )); then
		build
		run
		return
	fi

	local file="$1"

	printf 'File: %q\n' "$file"

	if [[ "$(realpath --physical -- "$file")" == "$script_path" ]]; then
		exec "$BASH" "$script_path" "${script_args[@]}"
	elif [[ "$file" != *'.lua' ]]; then
		build
	fi

	run
}

eval "$(luarocks path --lua-version "$lua_version")"

rebuild_and_run || true
while true; do
	rebuild_and_run "$(inotifywait \
		--quiet \
		--format '%w%f' \
		--event 'modify' \
		--include '\.(c|h|lua|sh)$' \
		-- './'
	)" || true
done
