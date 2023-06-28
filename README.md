# Reveal

## Starting Point

The Reveal project is terminal utility for Unix-like operating systems that
reveals information about entries in the file system. It was designed to work
perfectly with scripting tools, such as Shell scripting.

## Installation

This version is not available for production yet, but you can still install
previous versions by visiting the [releases page](https://github.com/skippyr/reveal/releases).

## Usage

### First Steps

Reveal expects the paths of entries of your file system. For example: you can
make it reveal the contents of your current directory:

```bash
reveal .
```

You can change the mode Reveal is operating by using flags. By default, Reveal
reveals the contents of the entries, but let's say, for example: you want to
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

### Examples

Reveal's output is very simple, but that is the perfect format for you to extend
it by using other commands available in your system. What about using a scripting
language such as Shell scripting?

Here are some cool examples for you to try it out:

-   List the contents of directories alphabetically.

```bash
function reveal-ls {
  typeset IFS=$'\n'
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(".")
  for p in ${paths[@]}; do
    if [[ ! -d $p ]]; then
      echo "$0: \"$p\" is not a directory."
      continue
    fi
    echo $p:
    for e in $(reveal $p); do
      echo "  ${e##*/}"
    done
    echo
  done | fmt
}
```

## Issues And Contributions

Learn how to report issues and contribute to this project by reading its
[contributions guidelines](https://skippyr.github.io/materials/pages/contributions_guidelines.html).

## License

This project is released under the terms of the MIT license. A copy of the
license is bundled with the source code.

Copyright (c) 2023, Sherman Rofeman. MIT license.
