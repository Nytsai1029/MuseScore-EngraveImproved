#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Runs an autobot script in the graphical interface of the installed app, without a screen,
# so that the interface can be operated and looked at (screenshots) from the command line.
# The app runs in a profile of its own (MUSE_APP_PROFILE): the settings, the user data, the recent files
# and the running instances of the user are not touched.

set -euo pipefail

readonly HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT="$(cd "${HERE}/../.." && pwd)"

readonly PROFILE="${UITEST_PROFILE:-uitest}"
readonly SKIN="${UITEST_SKIN:-atelier}"
readonly THEME="${UITEST_THEME:-dark}"
readonly LANGUAGE="${UITEST_LANGUAGE:-zh_CN}"
readonly SCREEN_SIZE="${UITEST_SCREEN_SIZE:-1600x1000}"
readonly DPR="${UITEST_DPR:-1}"
readonly TIMEOUT="${UITEST_TIMEOUT:-120}"
readonly MSCORE_BIN="${UITEST_MSCORE:-${ROOT}/build.install/mscore.app/Contents/MacOS/mscore}"

function usage() {
    printf '%s\n' \
        "Usage: ${0} <script.js> [score file]" \
        "" \
        "The script gets the folder for its results as api.context.globalVal(\"out_dir\")" \
        "(tmp/uitest/<name of the script>, or UITEST_OUT)." \
        "" \
        "Environment:" \
        "  UITEST_OUT          folder for the results (it is emptied first)" \
        "  UITEST_PROFILE      profile of the app [${PROFILE}]" \
        "  UITEST_SKIN         interface skin: default, dorico, atelier [${SKIN}]" \
        "  UITEST_THEME        theme: light, dark [${THEME}]" \
        "  UITEST_LANGUAGE     interface language [${LANGUAGE}]" \
        "  UITEST_SCREEN_SIZE  size of the screen which the app sees [${SCREEN_SIZE}]" \
        "  UITEST_DPR          device pixel ratio of that screen (2: a Retina screen) [${DPR}]" \
        "  UITEST_TIMEOUT      seconds after which the app is killed [${TIMEOUT}]" \
        "  UITEST_MSCORE       the app to run [build.install/…/mscore]"
}

# The settings of the profile, so that the app starts in the wanted skin without the first launch setup.
# The settings of a macOS app are found by its name, hence by its profile.
function write_settings() {
    if [[ "$(uname)" != "Darwin" ]]; then
        printf 'Note: the settings of the profile are only prepared on macOS\n' >&2
        return
    fi

    local -r domain="org.musescore.MuseScore4Development-${PROFILE}"
    defaults write "${domain}" "application.hasCompletedFirstLaunchSetup" -bool true
    defaults write "${domain}" "language" -string "${LANGUAGE}"
    defaults write "${domain}" "ui.application.currentSkinCode" -string "${SKIN}"
    defaults write "${domain}" "ui.application.currentThemeCode" -string "${THEME}"
}

function main() {
    if [[ "$#" -lt 1 || "${1}" == "-h" || "${1}" == "--help" ]]; then
        usage
        exit 1
    fi

    local -r script="$(cd "$(dirname "${1}")" && pwd)/$(basename "${1}")"
    local -r score="${2:-}"
    local -r out_dir="${UITEST_OUT:-${ROOT}/tmp/uitest/$(basename "${script}" .js)}"
    local -r log="${out_dir}/app.log"

    rm -rf "${out_dir}"
    mkdir -p "${out_dir}"
    printf '{ "out_dir": "%s" }\n' "${out_dir}" > "${out_dir}/context.json"

    # Qt reads the path of this file as Latin-1, so it is kept out of the folders of the user,
    # whose names may have other characters
    local -r screen_config="$(mktemp "${TMPDIR:-/tmp}/uitest-screen.XXXXXX")"
    trap 'rm -f "${screen_config}"' EXIT
    #! NOTE Windows derive their scale factor from logicalDpi / logicalBaseDpi (QHighDpiScaling);
    # the "dpr" key alone only changes QScreen::devicePixelRatio, not the raster pipeline.
    # width/height are native pixels, so the logical size stays UITEST_SCREEN_SIZE at any DPR.
    printf '{ "screens": [ { "name": "uitest", "x": 0, "y": 0, "width": %s, "height": %s, "dpr": %s, "logicalDpi": %s, "logicalBaseDpi": 96 } ] }\n' \
        "$((${SCREEN_SIZE%x*} * DPR))" "$((${SCREEN_SIZE#*x} * DPR))" "${DPR}" "$((96 * DPR))" > "${screen_config}"

    write_settings

    local args=(--test-case "${script}" --test-case-gui --test-case-context "${out_dir}/context.json")
    if [[ -n "${score}" ]]; then
        args+=("${score}")
    fi

    # The software renderer of Qt Quick needs no graphics device
    set +e
    env MUSE_APP_PROFILE="${PROFILE}" \
        QT_QPA_PLATFORM="offscreen:configfile=${screen_config}" \
        QT_QUICK_BACKEND=software \
        "${MSCORE_BIN}" "${args[@]}" > "${log}" 2>&1 &
    local -r app_pid=$!

    ( sleep "${TIMEOUT}"; kill -9 "${app_pid}" 2> /dev/null && printf 'TIMEOUT after %s s\n' "${TIMEOUT}" >> "${log}" ) &
    local -r watchdog_pid=$!

    wait "${app_pid}" 2> /dev/null
    local -r code=$?
    kill "${watchdog_pid}" 2> /dev/null
    wait "${watchdog_pid}" 2> /dev/null
    set -e

    # What went wrong, without the noise which every run of the app has
    sed -E 's/\x1b\[[0-9;]*m//g' "${log}" \
        | grep -E 'ERROR|ASSERT FAILED|TIMEOUT|failed step|\.qml:[0-9]+|TypeError|ReferenceError' \
        | grep -vE 'plugins\.js|sessionProjectsPaths|audiosettings\.json|viewsettings\.json|resolveSynth|MigrationContentForPre362' \
        | cut -c1-300 || true

    printf 'Exit code: %s\nResults: %s\n' "${code}" "${out_dir}"
    find "${out_dir}" -name '*.png' | sort
    exit "${code}"
}

main "$@"
