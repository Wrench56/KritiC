#!/bin/sh
set -e

if [ -z "$1" ]; then
  echo "Usage: $0 <version> (e.g. 1.2.3)"
  exit 1
fi

VERSION="$1"

case "$VERSION" in
  [0-9]*.[0-9]*.[0-9]*) ;;
  *)
    echo "Invalid version format. Expected X.Y.Z"
    exit 1
    ;;
esac

MAJOR=$(printf %s "$VERSION" | cut -d. -f1)
MINOR=$(printf %s "$VERSION" | cut -d. -f2)
PATCH=$(printf %s "$VERSION" | cut -d. -f3)

HEADER_FILE="kritic.h"

sed -i \
  -e "s/^#define[[:space:]]*KRITIC_VERSION_MAJOR[[:space:]]*[0-9]\+/#define KRITIC_VERSION_MAJOR $MAJOR/" \
  -e "s/^#define[[:space:]]*KRITIC_VERSION_MINOR[[:space:]]*[0-9]\+/#define KRITIC_VERSION_MINOR $MINOR/" \
  -e "s/^#define[[:space:]]*KRITIC_VERSION_PATCH[[:space:]]*[0-9]\+/#define KRITIC_VERSION_PATCH $PATCH/" \
  "$HEADER_FILE"

echo "Updated $HEADER_FILE to version $VERSION"
