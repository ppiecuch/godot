
_get_cpu () {
	if command -v getconf &> /dev/null; then
		echo $(getconf _NPROCESSORS_ONLN)
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo $(sysctl -n hw.physicalcpu)
	elif [[ "$OSTYPE" == "linux"* ]]; then
		echo $(nproc)
	else
		echo 2
	fi
}

_run_in_docker () {
	image="$1"
	script="$2"

	# toolchain not found - run docker image
	if ! command -v docker &> /dev/null
	then
		echo "*** Docker is not found - cannot run build script."
		exit 1
	fi
	oldopt=$- && set +e
	docker_state=$(docker info >/dev/null 2>&1)
	if [[ $? -ne 0 ]]; then
		echo "*** Docker does not seem to be running, run it first."
		exit 1
	fi

	set -$oldopt

	APPDIR="$(cd "$PWD" && pwd)"
	SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	if [[ -z $script ]]; then
		script="$(basename "${BASH_SOURCE[0]}")"
	fi

	echo "*** Running docker toolchain ${image} (with script $NAME).."
	docker run --rm -t -v "$APPDIR:/app" ${image} "./${SCRIPTDIR/$APPDIR/}/$script"

	exit
}

# Sync CLAUDE.md and CLAUDE-NOTES.md between this machine and the two known hosts.
# The newest copy wins on each pair. Current machine may be one of the hosts or a third machine.
sync_extra () {
	local REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
	local FILES=("CLAUDE.md" "CLAUDE-NOTES.md" "modules/gdextensions/submodules/albmpgfx/NOTES.md" "modules/gdextensions/submodules/daedalus/README.md" "modules/gdextensions/submodules/silentwolf/README.md")

	# host:path pairs for sync targets
	local TARGETS=(
		"192.168.1.17:~/Private/Software/GodotEngine/godot"
		"192.168.1.199:/Volumes/WORKSPACE/build-private/GodotEngine/godot-3.x"
	)

	# Get local IPs to avoid rsyncing to ourselves
	local LOCAL_IPS
	LOCAL_IPS=$(ip -4 -o addr show 2>/dev/null | awk '{print $4}' | cut -d/ -f1 || true)
	LOCAL_IPS+=$'\n'$(ifconfig 2>/dev/null | awk '/inet /{print $2}' || true)

	for target in "${TARGETS[@]}"; do
		local host="${target%%:*}"
		local remote_root="${target#*:}"

		# Skip if this host is us
		if echo "$LOCAL_IPS" | grep -qxF "$host"; then
			continue
		fi

		# Verify host is reachable
		if ! ssh -o ConnectTimeout=3 -o BatchMode=yes "$host" true 2>/dev/null; then
			echo "*** sync_extra: ${host} unreachable, skipping."
			continue
		fi

		echo "*** sync_extra: syncing with ${host}:${remote_root}"

		for f in "${FILES[@]}"; do
			local LOCAL_FILE="${REPO_ROOT}/${f}"
			local REMOTE_FILE="${remote_root}/${f}"

			local LOCAL_MTIME=0
			if [[ -f "$LOCAL_FILE" ]]; then
				if [[ "$OSTYPE" == "darwin"* ]]; then
					LOCAL_MTIME=$(stat -f %m "$LOCAL_FILE")
				else
					LOCAL_MTIME=$(stat -c %Y "$LOCAL_FILE")
				fi
			fi

			local REMOTE_MTIME
			REMOTE_MTIME=$(ssh "$host" "stat -f %m '${REMOTE_FILE}' 2>/dev/null || stat -c %Y '${REMOTE_FILE}' 2>/dev/null || echo 0") || REMOTE_MTIME=0

			if [[ "$LOCAL_MTIME" -gt "$REMOTE_MTIME" ]]; then
				echo "  ${f}: pushing to ${host} (local is newer)"
				rsync -pt "$LOCAL_FILE" "${host}:${REMOTE_FILE}"
			elif [[ "$REMOTE_MTIME" -gt "$LOCAL_MTIME" ]]; then
				echo "  ${f}: pulling from ${host} (remote is newer)"
				rsync -pt "${host}:${REMOTE_FILE}" "$LOCAL_FILE"
			else
				echo "  ${f}: in sync"
			fi
		done
	done
}
