# revelio
A program that reveals info about file system entries on Linux, such as: their
contents, type, permissions, ownership and modified date.

## Requirements
In order to build it, you will only need a C99 compiler, like `gcc`, and `make`.

## Install
Edit the file `config.mk` to match your needs. Then, run the following command
to install it (use `sudo` if necessary):

```sh
make install
```

## Uninstall
If you want to uninstall it, run the following command (use `sudo` if
necessary):

```sh
make uninstall
```

## Documentation
Once installed, you can read its manual.

```sh
man revelio.1
```

## Copyright
See `LICENSE` for copyright and license details.
