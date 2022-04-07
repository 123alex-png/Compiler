#!/bin/bash

gdb  ./parser \
 -ex "break VarDec"\
 -ex "run ~/test_l2/tests/m22.cmm"