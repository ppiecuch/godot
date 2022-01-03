
_get_cpu () {
	if command -v getconf &> /dev/null
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

	# toolchain not found - run docker image
	if ! command -v docker &> /dev/null
	then
		echo "*** Docker is not found - cannot run build script."
		exit 1
	fi
	docker_state=$(docker info >/dev/null 2>&1)
	if [[ $? -ne 0 ]]; then
		echo "*** Docker does not seem to be running, run it first."
		exit 1
	fi

	APPDIR="$(cd "$PWD" && pwd)"
	SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	NAME="$(basename "${BASH_SOURCE[0]}")"

	echo "*** Running docker toolchain ${image} (with script $NAME).."
	docker run --rm -t -v "$APPDIR:/app" ${image} "./${SCRIPTDIR/$APPDIR/}/$NAME"

	exit
}
