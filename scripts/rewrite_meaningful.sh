#!/bin/bash

# Script to rewrite git commit messages to be more meaningful and conventional

set -e

echo "Rewriting git history with more meaningful commit messages..."

git filter-branch --force --msg-filter '
    # Read the entire message
    msg=$(cat)

    # Get the first line
    first_line=$(echo "$msg" | head -n1)

    # Map old messages to new meaningful ones
    case "$first_line" in
        "initial commit of distributed machine learning framework")
            new_first="feat: initial implementation of distributed ML framework"
            ;;
        "create license")
            new_first="docs: add MIT license"
            ;;
        "add improvement plan to main branch")
            new_first="docs: add planned improvements and TODO comments"
            ;;
        "merge pull request #1 from bniladridas/add-improvement-plan")
            new_first="feat: integrate improvement plan and TODO updates"
            ;;
        "create index.html")
            new_first="feat: add index.html page"
            ;;
        "feat: add citation file (.cff) (#6)")
            new_first="docs: add citation file (.cff)"
            ;;
        "feat: add conventional commit standards and scripts")
            new_first="feat: add conventional commit standards and scripts"
            ;;
        feat:\ initial\ implementation\ of\ distributed\ ML\ framework*)
            new_first="feat: initial implementation of distributed ML framework"
            ;;
        *)
            new_first="$first_line"
            ;;
    esac

    # Truncate to 30 characters if longer
    if [ ${#new_first} -gt 30 ]; then
        new_first=${new_first:0:30}
    fi

    # Output the new first line
    echo "$new_first"

    # Output the rest of the message (if any)
    echo "$msg" | tail -n +2
' -- --all

echo "History rewritten with meaningful messages. Force-push with: git push --force --all"