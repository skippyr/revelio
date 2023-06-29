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

### Examples

Reveal's output is very simple, but that is the perfect format for you to extend
it by using other commands available in your system. Like this, there is no
limit of what you can do.

Here are some cool examples for you to try it out in a POSIX-complient shell,
such as Bash or ZSH:

-   This function list the contents of directories alphabetically, similar to
    the `ls -A` command.

```bash
function reveal-ls {
  typeset IFS=$'\n'
  typeset paths=($@)
  [[ ${#paths[@]} -eq 0 ]] &&
    paths+=(.)
  for p in ${paths[@]}; do
    if [[ ! -d $p ]]; then
      echo -e "$0: \"$p\" is not a directory.\n"
      continue
    fi
    echo $p:
    for e in $(reveal $p | sort); do
      echo "  ${e##*/}"
    done
    echo
  done | fmt
}
```

-   Reveals directories recursively starting from the current directory, similar
    to the `tree` command.

```bash
function reveal-tree {
  typeset IFS=$'\n'
  function t {
    for e in $(reveal $1); do
      printf "│  %.0s" {0..$2}
      printf "├──"
      echo ${e##*/}
      [[ -d $e ]] &&
        t $e $(($2 + 1))
    done
  }
  t . 0
  unset -f t
}
```

## Issues And Contributions

Learn how to report issues and contribute to this project by reading its
[contributions guidelines](https://skippyr.github.io/materials/pages/contributions_guidelines.html).

## License

This project is released under the terms of the MIT license. A copy of the
license is bundled with the source code.

Copyright (c) 2023, Sherman Rofeman. MIT license.
