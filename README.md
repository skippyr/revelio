# Reveal (rvl)
A program that reveals info about file system entries on Linux, such as:
contents, type, permissions, ownership and modified date.

## Requirements
In order to build it, you will only need a C90 compiler, like `tcc`.

## Installation
- Compile the file `rvl.c`.

```bash
mkdir -p ~/.local/bin
tcc -o ~/.local/bin/rvl rvl.c
```

- Install the manual page `rvl.1`.

```bash
mkdir -p ~/.local/share/man/man1
cp rvl.1 ~/.local/share/man/man1
```

If necessary, add the `~/.local/bin` and `~/.local/share/man/man1` directories
to your system `PATH` and `MANPATH` environment variables, respectively, in
order to be able to access their files directly.

```bash
export PATH=${PATH}:~/.local/bin
export MANPATH=${MANPATH}:~/.local/share/man/man1
```

## Documentation
Once installed, you can read its manual.

```bash
man rvl.1
```

## Copyright
See `LICENSE` for copyright and license details.
