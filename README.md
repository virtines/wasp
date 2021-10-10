# Dir structure:

* `data`: data generated from experiments
* `figs`: graphs, plots
* `scripts`: experimental scripts, e.g. build toolchains or Jupyter notebooks
* `wasp`: Wasp source code and rust bindings
* `pass`: LLVM `virtine` keyword pass
* `httpc`: containerized HTTP handler demo


if you want to install *everything*, just run the `install.sh` script in the
root. This will install the `vcc` compiler to your path and install all the required
libraries needed for virtines and wasp. You can also uninstall everything with, you
guessed it, `uninstall.sh`
