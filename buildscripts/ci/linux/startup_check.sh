#!/usr/bin/env bash
# Runs the packaged app under a debugger on a virtual X server and checks that it survives startup.
# It runs the AppDir that the AppImage is made of, with the environment that the AppImage's AppRun sets up.
# If it crashes, the call stack (with symbols from the build tree) is printed to the job log and saved
# together with the app's own logs to the output directory.
#
# Exit code: 0 - the app was still running after the timeout
#            1 - the app crashed or exited during startup
#            2 - the check itself could not be run

APP_DIR="MuseScore"
SYMBOLS_DIR="build.release"
OUT_DIR="build.startup_check"
TIMEOUT_SEC=90

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --appdir) APP_DIR="$2"; shift ;;
        --symbols_dir) SYMBOLS_DIR="$2"; shift ;;
        --out_dir) OUT_DIR="$2"; shift ;;
        --timeout) TIMEOUT_SEC="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 2 ;;
    esac
    shift
done

APP_BIN="$(find "$APP_DIR/bin" -maxdepth 1 -type f -name "mscore4portable*" -print -quit 2>/dev/null)"
if [ -z "$APP_BIN" ]; then
    echo "::error::Startup check: not found $APP_DIR/bin/mscore4portable*"
    exit 2
fi
APP_BIN="$(readlink -f "$APP_BIN")"
APP_DIR="$(readlink -f "$APP_DIR")"

mkdir -p "$OUT_DIR"
OUT_DIR="$(readlink -f "$OUT_DIR")"

DEBUGGER_OUTPUT="$OUT_DIR/debugger_output.txt"
SUMMARY="$OUT_DIR/summary.txt"

# ============================
# Install the tools
# ============================
PACKAGES=()
command -v gdb >/dev/null 2>&1 || PACKAGES+=(gdb)
command -v xvfb-run >/dev/null 2>&1 || PACKAGES+=(xvfb xauth)
if [ ${#PACKAGES[@]} -gt 0 ]; then
    echo "Installing ${PACKAGES[*]}..."
    sudo apt-get install -y --no-install-recommends "${PACKAGES[@]}"
fi

if ! command -v xvfb-run >/dev/null 2>&1; then
    echo "::error::Startup check: xvfb-run not available"
    exit 2
fi

XVFB_RUN=(xvfb-run --auto-servernum --server-args="-screen 0 1920x1080x24")

# ============================
# Environment
# ============================
# The same as the AppImage's AppRun creates. Its fallback libraries are not needed here:
# they were copied from this machine, so the system has them.
export APPDIR="$APP_DIR"
export APPIMAGE="$APP_DIR/AppRun"

# Use only what is bundled in the AppDir, not the Qt installed for the build
unset QT_PLUGIN_PATH QML2_IMPORT_PATH QML_IMPORT_PATH LD_LIBRARY_PATH

# ============================
# Run
# ============================
TIMED_OUT=false

app_pids() {
    local proc
    for proc in /proc/[0-9]*; do
        if [ "$(readlink "$proc/exe" 2>/dev/null)" == "$APP_BIN" ]; then
            echo "${proc#/proc/}"
        fi
    done
}

# Runs the command in the background (output to the file given as the first argument) and waits until it ends
# or the timeout expires. If the app is still running then, TIMED_OUT is set and the app is killed.
run_and_wait() {
    local output="$1"
    shift

    "$@" >"$output" 2>&1 </dev/null &
    local pid=$!

    local elapsed=0
    while kill -0 "$pid" 2>/dev/null && [ "$elapsed" -lt "$TIMEOUT_SEC" ]; do
        sleep 1
        elapsed=$((elapsed + 1))
    done

    if kill -0 "$pid" 2>/dev/null; then
        local pids
        pids="$(app_pids)"
        if [ -n "$pids" ]; then
            TIMED_OUT=true
            kill -KILL $pids 2>/dev/null
        fi

        # Let the debugger print and quit
        for _ in $(seq 1 20); do
            kill -0 "$pid" 2>/dev/null || break
            sleep 1
        done
        kill -KILL "$pid" 2>/dev/null
    fi

    wait "$pid" 2>/dev/null
}

if command -v gdb >/dev/null 2>&1; then
    echo "Debugger: $(command -v gdb)"

    GDB_ARGS=(-q -batch -nx
        -iex "set debuginfod enabled off"
        -ex "set pagination off"
        -ex "set print thread-events off"
        -ex "handle all nostop noprint pass"
        -ex "handle SIGSEGV SIGABRT SIGBUS SIGFPE SIGILL SIGSYS stop print"
    )

    # linuxdeploy strips the binaries in the AppDir; the one in the build tree still has its symbols
    SYMBOLS_FILE="$(find "$SYMBOLS_DIR/src/app/" -type f -name "mscore4portable*" -print -quit 2>/dev/null)"
    if [ -n "$SYMBOLS_FILE" ] && ! file "$APP_BIN" | grep -q "not stripped"; then
        echo "Symbols: $SYMBOLS_FILE"
        GDB_ARGS+=(-ex "symbol-file $(readlink -f "$SYMBOLS_FILE")")
    fi

    # Stop only at a crash: then print the registers and the stacks and quit.
    # The commands after "run" are also executed if the process exits or is killed at the timeout.
    GDB_ARGS+=(
        -ex "run"
        -ex "echo \n===== REGISTERS =====\n"
        -ex "info registers"
        -ex "echo \n===== STACK OF THE FAULTING THREAD =====\n"
        -ex "bt 80"
        -ex "echo \n===== ALL THREADS =====\n"
        -ex "thread apply all bt 25"
        -ex "echo \n===== END =====\n"
        --args "$APP_BIN"
    )

    run_and_wait "$DEBUGGER_OUTPUT" "${XVFB_RUN[@]}" gdb "${GDB_ARGS[@]}"
else
    echo "::warning::Startup check: no debugger available, running the app without it (no call stack on a crash)"

    run_and_wait "$OUT_DIR/app_output.txt" "${XVFB_RUN[@]}" "$APP_BIN"
    echo "(no debugger)" >"$DEBUGGER_OUTPUT"
fi

# ============================
# Collect the app's own logs
# ============================
APP_LOGS_DIR="$OUT_DIR/app_logs"
mkdir -p "$APP_LOGS_DIR"

USER_DATA_ROOT="$HOME/.local/share/MuseScore"
if [ -d "$USER_DATA_ROOT" ]; then
    find "$USER_DATA_ROOT" -type f -name "*.log" | while read -r log; do
        cp "$log" "$APP_LOGS_DIR/$(basename "$(dirname "$(dirname "$log")")")_$(basename "$log")"
    done
fi

# ============================
# Report
# ============================
CRASHED=false

# The kill at the timeout (SIGKILL) is reported as "Program terminated with signal", not as a crash
if grep -Eq "Program received signal SIG(SEGV|ABRT|BUS|FPE|ILL|SYS|TRAP)" "$DEBUGGER_OUTPUT"; then
    CRASHED=true
fi
if grep -rqs "Application crashed with signal" "$APP_LOGS_DIR"; then
    CRASHED=true
fi

if $CRASHED; then
    RESULT="CRASHED during startup"
    EXIT_CODE=1
elif $TIMED_OUT; then
    RESULT="OK: still running after $TIMEOUT_SEC seconds"
    EXIT_CODE=0
else
    RESULT="EXITED during startup (no crash)"
    EXIT_CODE=1
fi

echo "================ debugger output ================"
cat "$DEBUGGER_OUTPUT"
echo "================ last app log ================"
LAST_APP_LOG="$(ls -t "$APP_LOGS_DIR"/*.log 2>/dev/null | grep -v "audiopluginregistration" | head -n 1)"
if [ -n "$LAST_APP_LOG" ]; then
    tail -n 40 "$LAST_APP_LOG"
fi
echo "================ result ================"
echo "Startup check: $RESULT"

echo "Startup check: $RESULT" >"$SUMMARY"

if [ -n "$GITHUB_STEP_SUMMARY" ]; then
    echo "### Startup check: $RESULT" >>"$GITHUB_STEP_SUMMARY"
fi

if [ "$EXIT_CODE" -ne 0 ]; then
    echo "::error::Startup check: $RESULT (see the debugger output in this step and the startup check artifact)"
fi

exit $EXIT_CODE
