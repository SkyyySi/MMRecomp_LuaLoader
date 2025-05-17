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

declare MOD_NAME='mm_recomp_shared_library_mod_template'
declare SHARED_LIB_NAME='MyLib'
declare SHARED_LIB_VERSION='1.0.0'

declare MODS_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/Zelda64Recompiled/mods"
declare SHARED_LIB_FILE="$SHARED_LIB_NAME-$SHARED_LIB_VERSION.so"
declare MOD_FILE="$MOD_NAME.nrm"

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

function stop_game() {
	printf '\n>>> Stopping currently running game instance(s)...\n' 1>&2
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
	{
		make clean &&
		make -j "$(nproc)" &&
		RecompModTool './mod.toml' './build' &&
		flatpak run \
			--command='/app/bin/Zelda64Recompiled' \
			'io.github.zelda64recomp.zelda64recomp'
	} &

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
			printf '>>> File change detected: %q\n' "$changed_file" 1>&2
			stop_game
			break
		elif (( exit_code == 1 )); then
			printf '>>> Failed to wait for source file changes!\n' 1>&2
			echo "$changed_file"
			exit 1
		elif (( exit_code == 2 )); then
			#printf '>>> Timeout reached, still waiting...\n' 1>&2
			continue
		fi
	done
done
