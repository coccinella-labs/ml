#!/bin/bash

# Script to rewrite git commit messages for conventional commits
# Makes first line lowercase and truncates to ≤60 characters

set -e

echo "Rewriting git history to conform to conventional commit standards..."

git filter-branch --force --msg-filter '
    # Read the entire message
    msg=$(cat)

    # Get the first line
    first_line=$(echo "$msg" | head -n1)

    # Convert to lowercase
    first_line=$(echo "$first_line" | tr "[:upper:]" "[:lower:]")

    # Truncate to 60 characters if longer
    if [ ${#first_line} -gt 60 ]; then
        first_line=${first_line:0:60}
    fi

    # Output the modified first line
    echo "$first_line"

    # Output the rest of the message (if any)
    echo "$msg" | tail -n +2
' -- --all

echo "History rewritten. You can now force-push with: git push --force --all"