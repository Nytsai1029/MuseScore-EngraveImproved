#!/usr/bin/env bash
# Runs the freshly built app under a debugger and checks that it survives startup.
# If it crashes, the call stack is printed to the job log and saved together with the app's own logs
# (and any macOS crash report) to the output directory.
#
# Exit code: 0 - the app was still running after the timeout
#            1 - the app crashed or exited during startup
#            2 - the check itself could not be run

APP_BIN="applebuild/mscore.app/Contents/MacOS/mscore"
OUT_DIR="build.startup_check"
TIMEOUT_SEC=90

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --app_bin) APP_BIN="$2"; shift ;;
        --out_dir) OUT_DIR="$2"; shift ;;
        --timeout) TIMEOUT_SEC="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 2 ;;
    esac
    shift
done

if [ ! -x "$APP_BIN" ]; then
    echo "::error::Startup check: not found $APP_BIN"
    exit 2
fi
APP_BIN="$(cd "$(dirname "$APP_BIN")" && pwd)/$(basename "$APP_BIN")"
APP_NAME="$(basename "$APP_BIN")" # process name

mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"

DEBUGGER_OUTPUT="$OUT_DIR/debugger_output.txt"
SUMMARY="$OUT_DIR/summary.txt"

START_MARKER="$OUT_DIR/.started"
touch "$START_MARKER"

# ============================
# Run
# ============================
TIMED_OUT=false

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
        if pgrep -x "$APP_NAME" >/dev/null; then
            TIMED_OUT=true
        fi

        pkill -KILL -x "$APP_NAME"

        # Let the debugger print and quit
        for _ in $(seq 1 20); do
            kill -0 "$pid" 2>/dev/null || break
            sleep 1
        done
        kill -KILL "$pid" 2>/dev/null
    fi

    wait "$pid" 2>/dev/null
}

DEBUGGER_USED=false

if command -v lldb >/dev/null 2>&1; then
    echo "Debugger: $(command -v lldb)"

    # Stop only at a crash: then print the registers and the stacks and quit.
    # The commands after "-k" run if the process stops for any reason (including the kill at the timeout).
    run_and_wait "$DEBUGGER_OUTPUT" lldb --batch \
        -o "settings set auto-confirm true" \
        -o "process handle --pass true --stop false --notify false SIGPIPE SIGCHLD SIGUSR1 SIGUSR2 SIGALRM SIGVTALRM SIGPROF SIGWINCH SIGIO SIGURG" \
        -o "run" \
        -k "register read" \
        -k "thread backtrace -c 80" \
        -k "thread backtrace all -c 25" \
        -k "process kill" \
        -k "quit" \
        -- "$APP_BIN"

    if grep -Eq "Process [0-9]+ launched" "$DEBUGGER_OUTPUT"; then
        DEBUGGER_USED=true
    else
        echo "::warning::Startup check: the debugger could not start the app, running the app without it (no call stack on a crash)"
        mv "$DEBUGGER_OUTPUT" "$OUT_DIR/debugger_errors.txt"
    fi
else
    echo "::warning::Startup check: no debugger available, running the app without it (no call stack on a crash)"
fi

if ! $DEBUGGER_USED; then
    TIMED_OUT=false
    run_and_wait "$OUT_DIR/app_output.txt" "$APP_BIN"
    echo "(no debugger)" >"$DEBUGGER_OUTPUT"
fi

# ============================
# Collect the app's own logs and crash reports
# ============================
APP_LOGS_DIR="$OUT_DIR/app_logs"
mkdir -p "$APP_LOGS_DIR"

USER_DATA_ROOT="$HOME/Library/Application Support/MuseScore"
if [ -d "$USER_DATA_ROOT" ]; then
    find "$USER_DATA_ROOT" -type f -name "*.log" | while read -r log; do
        cp "$log" "$APP_LOGS_DIR/$(basename "$(dirname "$(dirname "$log")")")_$(basename "$log")"
    done
fi

# macOS writes them when the app crashes without a debugger attached
CRASH_REPORTS_DIR="$OUT_DIR/crash_reports"
mkdir -p "$CRASH_REPORTS_DIR"
find "$HOME/Library/Logs/DiagnosticReports" -maxdepth 1 -type f -name "${APP_NAME}*" -newer "$START_MARKER" \
    -exec cp {} "$CRASH_REPORTS_DIR/" \; 2>/dev/null
rm -f "$START_MARKER"

# ============================
# Report
# ============================
CRASHED=false

# The kill at the timeout (SIGKILL) is not a crash
if grep -Eq "stop reason = (EXC_[A-Z_]+|signal SIG(SEGV|ABRT|BUS|FPE|ILL|TRAP|SYS))" "$DEBUGGER_OUTPUT"; then
    CRASHED=true
fi
if grep -rqs "Application crashed with signal" "$APP_LOGS_DIR"; then
    CRASHED=true
fi
if [ -n "$(ls -A "$CRASH_REPORTS_DIR")" ]; then
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
for report in "$CRASH_REPORTS_DIR"/*; do
    [ -f "$report" ] || continue
    echo "================ crash report: $(basename "$report") ================"
    head -c 30000 "$report"
    echo
done
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
