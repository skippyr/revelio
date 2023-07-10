# Reveal

Reveal is a terminal utility that can reveal information about entries of
Unix-like operating system's file systems. It can be considered as a more
portable version of other similar commands, such as: `ls`, `find`, `cat` and
`stat`. Its simple outputs also allows you to extend it to your liking and needs
using scripting languages.

## Installation

Before installing it, download the following dependencies:

-   `git`: this program will be used to download this repository.
-   `clang` and `make`: these programs will be used to compile the source code.

Now that you have everything ready, follow these steps:

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

-   Add the binary `reveal` to a directory tracked in your system's `PATH`
    variable to make it available as command in your shell.

## Usage

### First Use

Reveal expects a list of entries of the file system. By default, it will
reveal their contents. For example: you can make it reveal the entries in
your current directory.

```bash
reveal .
```

### Data Type Flags

You can change the data type you want to retrieve by using a data type flag.
For example: you can make it reveal the size in bytes of an entry using `--size`.

```bash
reveal --size .
```

These data type flags will affect the output of any argument that follows them.
Like this, you can retrieve different data types from different entries at once.

```bash
reveal -size . --human-permissions . /
```

### Transpassing Flags

By default, Reveal does not resolve symlinks. If you want to change this
behavior, use the `--transpass` flag.

```bash
reveal --transpass /snap
```

Similarly to the data type flags, this flag will also affect any argument that
follows it.

### Help

You can get a list of available flags by reading its help page.

```bash
reveal --help
```

## Issues And Suggestions

Report issues and suggestion in the [issues tab](https://github.com/skippyr/reveal/issues).

## License

This project is released under the terms of the MIT license. A copy of the
license is bundled with the source code.

Copyright (c) 2023, Sherman Rofeman. MIT license.

