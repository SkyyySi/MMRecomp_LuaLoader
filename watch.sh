#!/usr/bin/env bash
# Zero-Clause BSD
# =============
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
# FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
# AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

clear

set -euCo pipefail
cd "$(dirname -- "${BASH_SOURCE[0]}")"

declare MOD_NAME='lua_loader'
declare SHARED_LIB_NAME='LuaLoader'
declare SHARED_LIB_VERSION='1.0.0'

declare MODS_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/Zelda64Recompiled/mods"
declare SHARED_LIB_FILE="$SHARED_LIB_NAME-$SHARED_LIB_VERSION.so"
declare MOD_FILE="$MOD_NAME.nrm"

# Position (1, 1) is the top-left corner. Both values must always be greater
# than or equal to one, otherwise something has gone very wrong.
function get_cursor_position() {
	if [[ ! -t 0 ]]; then
		return 1
	fi

	local -n nameref_line="$1"
	local -n nameref_column="$2"

	stty raw -echo

	printf '\e[6n' 1>&2

	local raw_cursor_position=''
	# This will read in `$'\e[L;C'`, where `L` and `C` are positive integers
	# representing the cursor's current line and column, respectively.
	read -d 'R' -r -s raw_cursor_position

	stty -raw echo

	# Strip the first opening square bracket and everything before it, turning
	# a string like `$'\e[L;C'` into `'L;C'`.
	local cursor_position="${raw_cursor_position#*\[}"

	# `${var%;*}` expands to everything before the last semicolon, while
	# `${var#*;}` expands to everything after the first semicolon. Example:
	#
	# ```bash
	# $ declare test='1;2;3'
	# $ echo "${test%;*}" "${test#*;}"
	# 1;2 2;3
	# ```
	# shellcheck disable=SC2034
	nameref_line=$(( ${cursor_position%;*} ))
	# shellcheck disable=SC2034
	nameref_column=$(( ${cursor_position#*;} ))
}

function reset_cursor_column() {
	local -i line=0
	local -i column=0
	get_cursor_position line column

	if (( line < 1 || column < 1 )); then
		return 1
	elif (( column == 1 )); then
		return 0
	fi

	printf '\n' 1>&2
}

function log() {
	local format="$1"
	shift
	local -a args=( "$@" )

	reset_cursor_column

	# shellcheck disable=SC2059
	printf '\e[1;35m>>>\e[22;39m '"$format"'\e[0m\n' "${args[@]}" 1>&2
}

if [[ ! -d "$MODS_DIR" ]]; then
	exit 1
fi

ln --symbolic --force --verbose -- "$PWD/build/src/shared/$SHARED_LIB_FILE" "$MODS_DIR"
ln --symbolic --force --verbose -- "$PWD/build/$MOD_FILE" "$MODS_DIR"

if ! command -v RecompModTool &> '/dev/null'; then
	if [[ ! -x './RecompModTool' ]]; then
		wget 'https://github.com/N64Recomp/N64Recomp/releases/download/mod-tool-release/RecompModTool' \
			--output-document './RecompModTool'

		chmod +x './RecompModTool'
	fi

	function RecompModTool() {
		'./RecompModTool' "$@"
	}
fi

function rebuild_game() {
	log 'Rebuilding game...'

	make clean || return 1
	make -j "$(nproc)" || return 1
	RecompModTool './mod.toml' './build' || return 1

	log 'Starting game...'

	{
		#flatpak run \
		#	--command='/app/bin/Zelda64Recompiled' \
		#	'io.github.zelda64recomp.zelda64recomp'
		(
			cd "$HOME/Zelda64Recompiled" &&
			env \
				LD_LIBRARY_PATH="${XDG_CONFIG_HOME:-$HOME/.config}/Zelda64Recompiled/mods" \
				'./Zelda64Recompiled'
		)

		local -i exit_code="$?"

		if (( exit_code != 0 )); then
			log 'The game terminated with non-zero exit code %d!' "$exit_code"
			return "$exit_code"
		fi

		log 'The game has been closed.'
	} &
}

function stop_game() {
	log 'Stopping currently running game instance(s)...'
	killall \
		--exact \
		--quiet \
		--signal='KILL' \
		-- \
		'Zelda64Recompiled'
}

function handle_sigint() {
	stop_game

	exit 0
}

# Stop the game when stopping this script via CTRL+C
trap handle_sigint SIGINT

declare changed_file=''
declare -i exit_code=0

set +e

while true; do
	rebuild_game

	while true; do
		changed_file=$(inotifywait \
			--quiet \
			--recursive \
			--format='%w%f' \
			--event='modify' \
			--include='\.[ch]$' \
			--timeout='1' \
			-- \
			'./src'
		)
		exit_code=$?

		if (( exit_code == 0 )); then
			clear
			log 'File change detected: %q' "$changed_file"
			stop_game
			break
		elif (( exit_code == 1 )); then
			log 'Failed to wait for source file changes!'
			echo "$changed_file"
			exit 1
		elif (( exit_code == 2 )); then
			#log 'Timeout reached, still waiting...'
			continue
		fi
	done
done
