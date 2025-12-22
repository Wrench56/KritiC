#!/bin/sh

# repo-init main script

# shellcheck disable=SC1090,SC2153,SC2154,SC2181

set -e

CONFIG_FILE="./repo.conf"
DEFAULT_REPO_URL="https://raw.githubusercontent.com/Wrench56/repo-init/main"

# ANSI color codes
if [ -t 1 ]; then
    ERROR_NODE="[\033[31m!\033[0m]"
    END_NODE="[\033[32m$\033[0m]"
    INFO_NODE="[\033[34m*\033[0m]"
    OUTP_NODE="[\033[35m@\033[0m]"
else
    ERROR_NODE="[!]"
    END_NODE="[$]"
    INFO_NODE="[*]"
    OUTP_NODE="[@]"
fi


# Download files using curl or wget
download_file() {
    url="$1"
    dest="$2"

    if command -v curl >/dev/null 2>&1; then
        curl -fsSL -o "$dest" "$url" && return 0
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "$dest" "$url" && return 0
    else
        printf "%b Error: Neither 'curl' nor 'wget' is available. Cannot download %s.\n" "$ERROR_NODE" "$url" >&2
        exit 3
    fi
}

# Format the output of a called tool
format_tool_output() {
    . "$@" 2>&1 | while IFS= read -r line; do
        printf "%b    %s\n" "$OUTP_NODE" "$line"
    done
}


# Load configuration
if [ -f "$CONFIG_FILE" ]; then
    . "$CONFIG_FILE"
else
    printf "%b Error: No configuration found!\n" "$ERROR_NODE" >&2
    exit 1
fi

# Ensure required config variables are set
: "${TOOLS_DIR:?Error: TOOLS_DIR is not defined in repo.conf}"
: "${TOOLS_LIST:?Error: TOOLS_LIST is not defined in repo.conf}"

# Ensure TOOLS_DIR exists
mkdir -p "$TOOLS_DIR" || {
    printf "%b Error: Could not create tools directory '%s'.\n" "$ERROR_NODE" "$TOOLS_DIR" >&2
    exit 2
}

printf "%b Processing tools...\n" "$INFO_NODE"
echo "$TOOLS_LIST" | grep -vE '^\s*#|^\s*$' | while read -r tool; do
    TOOL_SYSTEM=$(echo "$tool" | cut -d '-' -f1)
    TOOL_SCRIPT="$tool.sh"

    # Check if script exists, otherwise download it
    if [ ! -f "$TOOLS_DIR/$TOOL_SCRIPT" ]; then
        TOOL_URL="$DEFAULT_REPO_URL/$TOOL_SYSTEM/$TOOL_SCRIPT"
        printf "%b Downloading %s...\n" "$INFO_NODE" "$TOOL_SCRIPT"

        download_file "$TOOL_URL" "$TOOLS_DIR/$TOOL_SCRIPT"
        if [ $? -ne 0 ]; then
            printf "%b Error: Failed to download %s. Skipping...\n" "$ERROR_NODE" "$TOOL_SCRIPT" >&2
            continue
        fi
    fi

    chmod +x "$TOOLS_DIR/$TOOL_SCRIPT"
    printf "%b Executing %s...\n" "$INFO_NODE" "$tool"
    format_tool_output "$TOOLS_DIR/$TOOL_SCRIPT"
    rm -f "$TOOLS_DIR/$TOOL_SCRIPT"
    printf "%b Removed %s.\n" "$INFO_NODE" "$TOOL_SCRIPT"
done

printf "%b Setup complete!\n" "$END_NODE"
