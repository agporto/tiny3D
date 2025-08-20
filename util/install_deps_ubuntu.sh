#!/usr/bin/env bash
# Use: install_deps_ubuntu.sh [ assume-yes ] [ no-filament-deps ]

set -ev

SUDO=${SUDO:=sudo} # SUDO=command in docker (running as root, sudo not available)
options="$(echo "$@" | tr ' ' '|')"
APT_CONFIRM=""
if [[ "assume-yes" =~ ^($options)$ ]]; then
    APT_CONFIRM="--assume-yes"
fi

deps=(
    git
    # Tiny3D
    python3-dev
)

eval $(
    source /etc/lsb-release;
    echo DISTRIB_ID="$DISTRIB_ID";
    echo DISTRIB_RELEASE="$DISTRIB_RELEASE"
)

echo "apt-get install ${deps[*]}"
$SUDO apt-get update
$SUDO apt-get install ${APT_CONFIRM} ${deps[*]}
