---
tags:
  - contributing
---

# :handshake: Contributing

First off, thank you for considering contributing to FrameFi. It's people like you that make open source such a great community.

## :bug: Reporting Bugs

If you find a bug, please open an issue on our [GitHub Issues page][1].

Please include the following in your bug report:

- A clear and descriptive title.
- A detailed description of the problem, including steps to reproduce it.
- The expected behavior and what actually happened.
- Your hardware setup (e.g., board version, microSD card size).
- The version of the firmware you are running.

## :sparkles: Suggesting Enhancements

If you have an idea for a new feature or an improvement to an existing one, please open an issue on our [GitHub Issues page][1].

Please include the following in your enhancement suggestion:

- A clear and descriptive title.
- A detailed description of the proposed enhancement.
- Any relevant mockups or examples.

## :arrow_right_hook: Pull Requests

We welcome pull requests. If you'd like to contribute code, please follow these steps:

1.  Fork the repository.
2.  Create a new branch for your feature or bug fix.
3.  Make your changes, adhering to the project's coding and documentation standards.
4.  Submit a pull request with a clear description of your changes.

### :scroll: Code Style

This project follows the [GEMINI C++ Style Guide](../GEMINI.md). Please ensure your code adheres to these standards.

!!! tip "Key points"

    - Function names must be `lowerCamelCase`.
    - Every function must have a single-line documentation brief.
    - Implementation comments must use the `// --- comment text ---` style.

### :book: Documentation Style

All documentation is written in Markdown and generated using MkDocs with the Material theme. Please follow the [Markdown Documentation Guidelines](./GEMINI.md).

!!! tip "Key points"

    - Use clear and descriptive headings with emojis.
    - Use admonitions to highlight important information.
    - Specify the language for code blocks.
    - Add new pages to the `nav` section of `mkdocs.yml`.

### :label: Versioning

The firmware version is automatically generated based on the `git` history of the repository, so you don't need to set it manually. This ensures that each build is traceable to a specific point in the code's history.

The version string is derived using the `git describe --tags --dirty --always` command. Here's how it works:

-   **Tags:** The base version number comes from the most recent `git` tag (e.g., `v1.2.0`). It's crucial to tag releases in the format `vX.Y.Z`.
-   **Commit Hash:** If you have made commits since the last tag, the version will include the number of commits and the short hash of the latest commit (e.g., `v1.2.0-4-g1a2b3c4`). This indicates it's a development build.
-   **Dirty State:** If you have uncommitted changes in your local working directory when you build the firmware, the version string will have a `-dirty` suffix (e.g., `v1.2.0-4-g1a2b3c4-dirty`). This is a clear indicator that the build was made from code that doesn't exactly match any commit in the repository, which is useful for debugging.

This automated versioning helps maintain a clear and consistent understanding of what code is running on a device at any given time.

## :balance_scale: License

By contributing, you agree that your contributions will be licensed under the [Apache License 2.0][2].

## :link: References

- <https://github.com/nicholaswilde/frame-fi/blob/main/LICENSE>

[1]: <https://github.com/nicholaswilde/frame-fi/issues>
[2]: <./LICENSE>
