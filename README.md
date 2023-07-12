# Reveal

Reveal is a terminal utility that can reveal information about entries of
Unix-like file systems. Its simple outputs also allows you to extend it to
your liking and needs using scripting languages.

![](images/preview.gif)

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

Reveal expects a list of entries of the file system as arguments. By default,
it will reveal their contents. If you do not give an entry, it will consider
the last one given, else, the current directory.

As a first example, what about revealing your current directory:

```bash
reveal
```

And,  a file:

```bash
reveal LICENSE
```

### Data Type Flags

You can change the data type you want to retrieve by using a data type flag
before the entries you want to affect. For example, you can make it reveal the
size in bytes of your current directory using `--size`:

```bash
reveal --size
```

Or the human permissions of your home directory:

```bash
reveal --human-permissions ~
```

You can also stack multiple data type flags for multiple entries, allowing you
to retrive multiple data at once. For example, you can make it reveal the size
your current directory, the human permissions of the directories `/` and `~`,
and the mode of the directory `~`:

```bash
reveal --size --human-permissions / ~ --mode
```

### Transpassing Flags

By default, Reveal does not resolve symlinks. If you want to change this
behavior, use the `--transpass` flag before the entries you want to affect.
You can also counter this flag using the `--untranspass` flag. This allow you
to create interesting combos if you combine those with the data type flags.

For example: you can make it reveal the contents of what the symlink directory
`/snap` resolves to and also get the quantity of blocks the symlink itself
occupies.

```bash
reveal --transpass /snap --untranspass --blocks
```

### Help

For more details about the program and its features, visit its help page:

```bash
reveal --help
```

## Issues, Questions And Suggestions

Report issues, questions and suggestions in the [issues tab](https://github.com/skippyr/reveal/issues).

## License

This project is released under the terms of the MIT license. A copy of the
license is bundled with the source code.

Copyright (c) 2023, Sherman Rofeman. MIT license.

