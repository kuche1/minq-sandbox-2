#! /usr/bin/env bash

set -euo pipefail

HERE=$(dirname "$BASH_SOURCE")

COMPILER='gcc'

FLAGS_STANDARD='-std=c99'
# I'm purposefully using a different std form the library, to make sure that everything's file

FLAGS_STRICT='-Werror -Wextra -Wall -pedantic -Wfatal-errors -Wshadow'

FLAGS_LIBRARIES='-lseccomp'
# we can't have both `-static` and `-lseccomp`

FLAGS_OPTIMISATION='-Ofast'

FLAGS="$FLAGS_STANDARD $FLAGS_STRICT $FLAGS_LIBRARIES $FLAGS_OPTIMISATION"

clear

echo
echo 'Compiling libsandbox...'
echo

"$HERE/libsandbox/compile.sh"

echo
echo 'Compiling minq-sandbox-2...'
echo

$COMPILER $FLAGS -o "$HERE/minq-sandbox-2" "$HERE/src/minq-sandbox-2.c" "$HERE/libsandbox/libsandbox.a"

echo
echo 'All compiled'
echo
