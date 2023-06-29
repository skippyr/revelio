# Reveal

## Starting Point

The Reveal project is terminal utility for Unix-like operating systems that
reveals information about entries in the file system. It was designed to work
perfectly with scripting tools, such as Shell scripting.

![](images/preview.gif)

## Installation

This version is not available for production yet, but you can still install
previous versions by visiting the [releases page](https://github.com/skippyr/reveal/releases).

## Usage

Reveal expects the paths of entries of your file system. For example: you can
make it reveal the contents of your current directory:

```bash
reveal .
```

You can change the mode Reveal is operating by using flags. By default, Reveal
reveals the contents of the entries, but let's say, for example, you want to
know the size of a file: for that use the `--size` flag.

```bash
reveal --size foo.txt
```

You can also request for multiple types of data of multiple entries at once:
this one will print the contents and size of a file, the permissions of `/` and
the contents of the directory `/usr/bin`.

```bash
reveal foo.txt --size foo.txt --permissions / --contents /
```

For a full list of flags that you can use, check out its help:

```bash
reveal --help
```

## Extensions

Reveal's output is very simple, but that is the perfect format for you to extend
it by using other commands available in your system. Like this, there is no
limit of what you can do.

If you are using a POSIX-compliant shell such as Bash or ZSH you can take
advantage of some scripts I have made to extend Reveal's capabilities.
Those are called "extensions" and you can find them under the `extensions`
directory.

Check out, these example:

-   The `reveal-nf` extension adds Nerd Font icons when revealing
    directories:

![](images/preview_reveal_nf.gif)

-   The `reveal-tree` extension adds tree view format when revealing a
    directory.

![](images/preview_reveal_tree.gif)

-   The `reveal-ls` extension adds back the format that `ls` uses by default.

![](images/preview_reveal_ls.gif)

To use them, simple use a source command, for example:

```bash
source extensions/<EXTENSION>
```

Where `<EXTENSION>` must be replaced by the name of an extension file. Each
extension file will add a new function to your shell section that you can
use just like any other command. Read the extension file to understand what
each of them do.

As expected, Reveal allows you also create and share your own extensions.

## Issues And Contributions

Learn how to report issues and contribute to this project by reading its
[contributions guidelines](https://skippyr.github.io/materials/pages/contributions_guidelines.html).

## License

This project is released under the terms of the MIT license. A copy of the
license is bundled with the source code.

Copyright (c) 2023, Sherman Rofeman. MIT license.
