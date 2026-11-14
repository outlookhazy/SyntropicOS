#!/usr/bin/env bash
# Signal-safe container execution wrapper with pre-run cleanup & automatic CID tracking.

# Close leaked non-CLOEXEC file descriptors (>2) inherited from parent IDE/GUI processes
# so rootless Podman's pasta helper does not trigger SELinux FD-inheritance AVC alerts.
if [ -d "/proc/$$/fd" ]; then
    for _fd_path in /proc/$$/fd/*; do
        _fd="${_fd_path##*/}"
        if [ "$_fd" -gt 2 ] 2>/dev/null; then
            eval "exec ${_fd}>&-" 2>/dev/null || true
        fi
    done
    unset _fd_path _fd
fi

if [ -z "$CONTAINER_ENGINE" ]; then
    if command -v podman >/dev/null 2>&1; then
        CONTAINER_ENGINE="podman"
    else
        CONTAINER_ENGINE="docker"
    fi
fi
CONTAINER_NAME="syntropicos_run_$$"
CID_FILE="$(mktemp /tmp/syn_container_XXXXXX.cid)"
rm -f "$CID_FILE"

cleanup() {
    if [ -f "$CID_FILE" ]; then
        CID="$(cat "$CID_FILE" 2>/dev/null || true)"
        if [ -n "$CID" ]; then
            "$CONTAINER_ENGINE" rm -f "$CID" >/dev/null 2>&1 || true
        fi
        rm -f "$CID_FILE" 2>/dev/null || true
    fi
    "$CONTAINER_ENGINE" rm -f "$CONTAINER_NAME" >/dev/null 2>&1 || true
}

trap cleanup EXIT INT TERM HUP

# Pre-execution sweep: kill any leftover/orphaned containers from interrupted host tasks
if [[ "$CONTAINER_ENGINE" == *podman* ]]; then
    podman ps -q --filter "image=localhost/syntropicos-test" | xargs -r podman rm -f >/dev/null 2>&1 || true
fi

"$CONTAINER_ENGINE" run --init --name "$CONTAINER_NAME" --cidfile "$CID_FILE" --rm "$@"
EXIT_CODE=$?
cleanup
exit $EXIT_CODE
