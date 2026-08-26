#!/bin/sh
set -eu

tool_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
image=${1:-"$tool_dir/../build/peep.bin"}

command -v dfu-util >/dev/null 2>&1 || {
    echo "dfu-util is required" >&2
    exit 1
}
[ -f "$image" ] || {
    echo "missing firmware: $image" >&2
    exit 1
}

if ! dfu-util -d 0483:df11 -l 2>&1 | grep -qi '0483:df11'; then
    dfu-util -d cafe:5045 -a 0 -e
fi

tries=0
while ! dfu-util -d 0483:df11 -l 2>&1 | grep -qi '0483:df11'; do
    tries=$((tries + 1))
    [ "$tries" -lt 50 ] || {
        echo "PEEP did not enter ROM DFU" >&2
        exit 1
    }
    sleep 0.1
done

dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D "$image"
