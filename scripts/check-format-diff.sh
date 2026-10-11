#!/usr/bin/env bash
# Fails if lines changed since <base> (default: origin/main) are not clang-format clean.
# Checks changed lines only: the legacy tree is not uniformly formatted, so whole-file checks would
# reject every PR that touches an old file.
set -euo pipefail

base="${1:-origin/main}"
status=0

while IFS= read -r file; do
    case "$file" in
        *.cpp | *.h) ;;
        *) continue ;;
    esac

    args=()
    # Hunk header: @@ -a,b +start,count @@ (count is omitted when 1, and 0 means pure deletion).
    while read -r start count; do
        count="${count:-1}"
        [ "$count" -eq 0 ] && continue
        args+=("--lines=${start}:$((start + count - 1))")
    done < <(git diff -U0 "${base}...HEAD" -- "$file" |
        sed -n 's/^@@ -[0-9,]* +\([0-9]*\),\{0,1\}\([0-9]*\) @@.*/\1 \2/p')

    [ "${#args[@]}" -eq 0 ] && continue
    clang-format --dry-run --Werror --style=file "${args[@]}" "$file" || status=1
done < <(git diff --name-only --diff-filter=ACMR "${base}...HEAD")

exit "$status"
