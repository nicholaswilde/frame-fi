#!/usr/bin/env bash
################################################################################
#
# release.sh
# ----------------
# Automates the versioning, tagging, and deployment process for FrameFi.
#
# @author Nicholas Wilde, 0xb299a622
# @date 25 Oct 2026
# @version 0.1.0
#
################################################################################

# Options
set -e
set -o pipefail

# Constants
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly RESET=$(tput sgr0)

# Logging function
function log() {
  local type="$1"
  local message="$2"
  local color="$RESET"

  case "$type" in
    INFO)
      color="$BLUE";;
    WARN)
      color="$YELLOW";;
    ERRO)
      color="$RED";;
  esac

  echo -e "${color}${type}${RESET}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}"
}

function check_dependencies() {
  if ! command -v git >/dev/null 2>&1; then
    log "ERRO" "git is not installed." >&2
    exit 1
  fi
  if ! command -v task >/dev/null 2>&1; then
    log "ERRO" "task is not installed." >&2
    exit 1
  fi
}

function main() {
  local action="${1:-patch}"

  check_dependencies

  log "INFO" "Fetching latest tags and commits from origin..."
  git fetch --tags origin
  git pull --rebase origin main

  local latest_tag
  latest_tag=$(git tag --sort=-v:refname 2>/dev/null | head -n 1)

  local current_ver
  if [ -n "$latest_tag" ]; then
    current_ver="${latest_tag#v}"
  elif [ -f version.txt ]; then
    current_ver=$(tr -d '[:space:]' < version.txt)
    current_ver="${current_ver#v}"
  else
    current_ver="0.17.0"
  fi

  IFS='.' read -ra ver_parts <<< "$current_ver"
  local major="${ver_parts[0]:-0}"
  local minor="${ver_parts[1]:-0}"
  local patch="${ver_parts[2]:-0}"

  local new_ver
  case "$action" in
    major)
      new_ver="$((major + 1)).0.0"
      ;;
    minor)
      new_ver="${major}.$((minor + 1)).0"
      ;;
    patch)
      new_ver="${major}.${minor}.$((patch + 1))"
      ;;
    v*)
      new_ver="${action#v}"
      ;;
    [0-9]*)
      new_ver="$action"
      ;;
    *)
      log "ERRO" "Unknown release action: $action. Use: patch, minor, major, or explicit version (e.g. 0.17.1)" >&2
      exit 1
      ;;
  esac

  local new_tag="v$new_ver"

  log "INFO" "Current version: ${latest_tag:-none ($current_ver)}"
  log "INFO" "Target version:  $new_tag"

  if git rev-parse "$new_tag" >/dev/null 2>&1; then
    log "ERRO" "Tag $new_tag already exists in git!" >&2
    exit 1
  fi

  log "INFO" "Running pre-release validation (task build)..."
  task build

  log "INFO" "Validation passed. Bumping version.txt..."
  echo "$new_tag" > version.txt
  git add version.txt
  git commit -m "chore(release): bump version to $new_tag"

  log "INFO" "Checking git status..."
  if [ -n "$(git status --porcelain)" ]; then
    log "ERRO" "Working directory not clean after bumping version." >&2
    exit 1
  fi

  log "INFO" "Tagging release..."
  git tag -a "$new_tag" -m "Release $new_tag"

  log "INFO" "Pushing atomically..."
  git push --atomic origin main "$new_tag"

  log "INFO" "Release $new_tag published successfully! Now run ./scripts/release_summary.py"
}

main "$@"
