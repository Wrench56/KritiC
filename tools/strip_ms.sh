#!/bin/sh
set -e

if [ -t 0 ] && [ -z "$1" ]; then
  echo "Usage: $0 <output-file>"
  echo "       or pipe into it: cat file | $0"
  exit 1
fi


strip_ms() {
  sed -E 's/[0-9]+(\.[0-9]+)?ms/0.0ms/g'
}

if [ -n "$1" ]; then
  if [ ! -f "$1" ]; then
    echo "File not found: $1"
    exit 1
  fi
  strip_ms < "$1"
else
  strip_ms
fi
