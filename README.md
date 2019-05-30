# AntIDoTe

Using **A**bstract **I**nterpretation to reason about
**D**ecision-**T**ree learning's robustness to
training-set-**poisoning** attacks.
Source is still under active development.

## Build Instructions

The top-level Makefile handles everything; simply run `make`.
This creates the main binary, written to `bin/`.

Unit tests (the [test/](test/) directory) can be built and run using `make test`.
Testing uses the open source [Catch](https://github.com/catchorg/Catch2) framework
whose license permits including its source in this repository
(as the single header file [test/catch2/catch.hpp](test/catch2/catch.hpp)).
