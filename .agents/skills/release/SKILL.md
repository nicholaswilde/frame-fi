---
name: release
description: Automates the versioning, tagging, and deployment process for the project using scripts/release.sh.
---
# /release [patch|minor|major|<version>]

Automates the versioning, tagging, and deployment process for FrameFi.

## Description
This skill handles the release process by executing the `scripts/release.sh` utility. The script will automatically determine the next version (defaults to patch increment, or accepts `minor`, `major`, or an explicit version), run pre-release validation (`task build`), bump `version.txt`, commit, tag, and push atomically to the remote repository.

## Protocol

1. Simply execute the script in the terminal:
   ```bash
   ./scripts/release.sh
   # or with specific bump type:
   ./scripts/release.sh minor
   ```
2. If it succeeds, the new version will be tagged and pushed to trigger the GitHub Actions release workflow (`ci.yaml`).
3. Once the draft release is created on GitHub, run `/release-summary` or `./scripts/release_summary.py` to populate release notes.
