# Guide for Commits, Tags, and Releases

This document outlines the standard procedures for contributing to this repository, ensuring a clean commit history and consistent versioning. Following these guidelines makes it easier to track changes, generate automated release notes, and maintain a clear project timeline.

## 1. Conventional Commits

We use Conventional Commits to standardize our commit messages. This standard provides a set of rules for creating an explicit commit history.

The commit message structure should be:

```
<type>(<scope>): <description>
```

**Type**

The `type` is a mandatory prefix that describes the nature of the change.

feat: A new feature or functionality.

fix: A bug fix.

docs: Documentation only changes.

style: Changes that do not affect the meaning of the code (e.g., whitespace, formatting, missing semicolons).

refactor: A code change that neither fixes a bug nor adds a feature.

perf: A code change that improves performance.

test: Adding missing tests or correcting existing tests.

chore: Changes to the build process or auxiliary tools and libraries.

Scope (Optional)
The scope provides context for the change, such as a component, filename, or feature. For example, feat(auth) or fix(footer).

Description
The description is a brief, imperative sentence that summarizes the change. Do not end with a period.

Examples
Correct:

feat: add user authentication

fix(header): fix mobile layout bug

docs: update README with installation instructions

Incorrect:

- fixed a bug
- updated some stuff
- Auth changes.

## 2. Git Tags & Semantic Versioning (SemVer)

We use Semantic Versioning (SemVer) for all releases. This standard uses a three-part version number: MAJOR.MINOR.PATCH.

- **PATCH:** Increment the patch version when you make backwards-compatible bug fixes.
- **MINOR:** Increment the minor version when you add new functionality in a backwards-compatible manner.
- **MAJOR:** Increment the major version when you make breaking changes.

Tagging Commands
Once you are ready to create a release, use the following commands:

```
# Create a new annotated tag (replace vX.Y.Z with the new version)
git tag -a v1.0.1 -m "Release v1.0.1"

# Push the tag to the remote repository
git push --tags
```

## 3. Release Summary Template
When creating a new release on GitHub, use the following Markdown template to provide a clear and organized summary of the changes.

```
# Release vX.Y.Z

**Date:** [YYYY-MM-DD]

### What's New

- **New Features:**
    * (List new features here, corresponding to `feat` commits)

- **Bug Fixes:**
    * (List bug fixes here, corresponding to `fix` commits)

- **Other Changes:**
    * (List other noteworthy changes here, corresponding to `refactor`, `docs`, `chore`, etc. commits)
```
