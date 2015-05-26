ORP for or1k
============

This is the root path for ORP-or1k. The `Makefile` in this path contains targets to install both the or1k development tools (such as GCC) as well as ORP for or1k.

Subdirectories formed by the `Makefile`:

* `build`, which is used to stage and perform builds
* `slash`, the install prefix for the or1k development tools (optional)
* `orp`, the install prefix for ORP


Building the tools and ORP
--------------------------

Follow these directions if you don't already have an or1k development environment setup.

If you haven't already, clone the or1k-gcc (https://github.com/openrisc/or1k-gcc) and or1k-src (https://github.com/openrisc/or1k-src) repositories into the `ORP/third-party` directory.

There is a two-step procedure for building and installing everything (both the or1k development tools and ORP for or1k). First we `source` the appropriate environment variables, then we `make all`:

    [ORP/platform/or1k]$ source devenv-settings.sh
    [ORP/platform/or1k]$ time make all

    ...
    real	33m57.574s
    user	26m56.317s
    sys	1m47.100s
    [ORP/platform/or1k]$ 

As described above, this will install the or1k development tools into `ORP/platform/or1k/slash` and the ORP libaries and binaries to `ORP/platform/or1k/orp`.


Building just ORP
-----------------

If you already have an or1k development environment setup, you can build and install ORP without building the tools by using the `all-orp` target:

    [ORP/platform/or1k]$ time make all-orp
    ...
    [ORP/platform/or1k]$ 

