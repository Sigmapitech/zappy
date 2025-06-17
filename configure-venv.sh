#!/usr/bin/env bash
set -e

ENV_DIR=venv

if [ -d "$ENV_DIR" ]; then
  if ! "$ENV_DIR/bin/pip" --version &>/dev/null; then
    echo "pip in '$ENV_DIR' is broken or missing." >&2
    rm -rf "$ENV_DIR"
  fi
fi

if command -v python3 &>/dev/null; then
  PYTHON=python3
elif command -v python &>/dev/null; then
  PYTHON=python
else
  echo "No python or python3 executable found." >&2
  exit 1
fi

$PYTHON -m venv $ENV_DIR >/dev/null && exit 0
# falling back to `virtualenv`
command -v virtualenv &>/dev/null && virtualenv "$ENV_DIR" && exit 0

echo "No fallback possible, cannot create a virtual env."
exit 1
