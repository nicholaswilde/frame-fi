---
name: release-summary
description: Generates a clean, professional GitHub release summary based on git logs using scripts/release_summary.py.
---
# /release-summary [range]

Generates a clean, professional GitHub release summary based on git logs.

## Description
This skill uses `scripts/release_summary.py` to retrieve git commit logs, format them into a structured markdown summary (Features, Bug Fixes, Improvements, Documentation), and updates the existing GitHub draft release.

## Protocol

1. Execute the Python script. If no range is provided, it automatically detects the latest two tags.
   ```bash
   ./scripts/release_summary.py [range]
   ```

2. **Special Thanks**: If the user asks to add special thanks (e.g. to a contributor or tester), pass `--thanks` (and optionally `--for` / reason):
   ```bash
   # Name and reason
   ./scripts/release_summary.py [range] --thanks "<Name>" --for "<reason>"

   # Or full message
   ./scripts/release_summary.py [range] --thanks "Huge thanks to **<Name>** for <reason>!"
   ```
   This generates the section using the standard special thanks shape:
   ```markdown
   ### 🙏 **Special Thanks**

   Huge thanks to **<Name>** for <reason>!
   ```

3. **Dry Run**: Preview output without modifying GitHub:
   ```bash
   ./scripts/release_summary.py [range] --dry-run
   ```

4. The script parses the commits, generates the structured markdown, and automatically runs `gh release edit` to update the draft release in-place.
