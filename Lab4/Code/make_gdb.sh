#!/bin/bash

gdb  ./parser \
 -ex "break main"\
 -ex "run ../Test/my_test.cmm"