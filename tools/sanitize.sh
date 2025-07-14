#!/bin/sh
set -e

if [ -t 0 ] && [ -z "$1" ]; then
  echo "Usage: $0 <output-file>"
  echo "       or pipe into it: cat file | $0"
  exit 1
fi

sanitize() {
  sed -E '
    s/less than [0-9]+(\.[0-9]+)?ms/0.0ms/g;
    s/[0-9]+(\.[0-9]+)?ms/0.0ms/g;
    /KritiC v[0-9]+\.[0-9]+\.[0-9]+/d;
    /^make\[[0-9]+\]/d;
    s|build/selftest\.exe|build/selftest|g;
  '
}

if [ -n "$1" ]; then
  if [ ! -f "$1" ]; then
    echo "File not found: $1"
    exit 1
  fi
  sanitize < "$1"
else
  sanitize
fi
