#!/bin/bash

gdb  ./parser \
 -ex "break main"\
 -ex "run ../Test/optional_example1.cmm"