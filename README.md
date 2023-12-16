# revelio
A program that reveals info about file system entries on Linux, such as:
contents, type, permissions, ownership and modified date.

## Requirements
In order to build it, you will only need a C compiler, like `tcc`.

## Installation
- Compile the file `revelio.c`.

```bash
mkdir -p ~/.local/bin
tcc -o ~/.local/bin/revelio revelio.c
```

- Install the manual page `revelio.1`.

```bash
mkdir -p ~/.local/share/man/man1
cp revelio.1 ~/.local/share/man/man1
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
man revelio.1
```

## Copyright
See `LICENSE` for copyright and license details.
