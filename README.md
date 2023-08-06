# Reveal
## About
A terminal program that reveals information about entries in the Linux's file system, such as: contents, type, size, permissions, ownership and modified date.
![](preview.png)

## Installation
### Dependencies
The following dependencies must be installed to run install this software properly:

-   `git`.
-   `clang`.
-   `make`

### Procedures
Using a command-line, follow these procedures:

-   Clone this repository.
```bash
git clone --depth=1 https://github.com/skippyr/reveal
```

-   Access the repository's directory.
```bash
cd reveal
```

-   Compile the source code.
```bash
make
```

-   Add the binary `reveal` created in a directory tracked in your system's `PATH` environment variable.
-   Reopen your terminal emulator.

## Usage
For help instructions, read its manual page:
```bash
reveal --help
```

## Copyright
This software is under the MIT license. A copy of the license is bundled with the source code.
