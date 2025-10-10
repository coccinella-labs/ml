#!/bin/bash

# Script to rewrite git commit messages to be more meaningful and conventional

set -e

echo "Rewriting git history with more meaningful commit messages..."

git filter-branch --force --msg-filter '
    # Read the entire message
    msg=$(cat)

    # Get the first line
    first_line=$(echo "$msg" | head -n1)

    # Map old messages to new meaningful ones (≤30 chars)
    case "$first_line" in
        "initial commit of distributed machine learning framework")
            new_first="feat: initial ML framework impl"
            ;;
        "create license")
            new_first="docs: add MIT license"
            ;;
        "add improvement plan to main branch")
            new_first="docs: add TODO comments"
            ;;
        "merge pull request #1 from bniladridas/add-improvement-plan")
            new_first="feat: integrate TODO updates"
            ;;
        "create index.html")
            new_first="feat: add index.html page"
            ;;
        "feat: add citation file (.cff) (#6)")
            new_first="docs: add citation file"
            ;;
        "feat: add conventional commit standards and scripts")
            new_first="feat: add commit standards"
            ;;
        "chore: remove redundant files index.html and requirements.txt")
            new_first="chore: remove redundant files"
            ;;
        "chore: update commit rules to 30 chars")
            new_first="chore: update rules to 30"
            ;;
        "docs: update rules to 30 chars")
            new_first="docs: update rules to 30"
            ;;
        feat:\ initial\ implementation\ o*)
            new_first="feat: initial ML framework"
            ;;
        docs:\ add\ planned\ improvement*)
            new_first="docs: add TODO comments"
            ;;
        feat:\ integrate\ improvement\ p*)
            new_first="feat: integrate TODO updates"
            ;;
        feat:\ add\ index.html\ page*)
            new_first="feat: add index.html page"
            ;;
        docs:\ add\ citation\ file\ \(\.cff\)*)
            new_first="docs: add citation file"
            ;;
        feat:\ add\ conventional\ commit*)
            new_first="feat: add commit standards"
            ;;
        chore:\ remove\ redundant\ files*)
            new_first="chore: remove redundant files"
            ;;
        chore:\ update\ commit\ rules\ to*)
            new_first="chore: update rules to 30"
            ;;
        docs:\ update\ rules\ to\ 30\ chars*)
            new_first="docs: update rules to 30"
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