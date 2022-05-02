#!/bin/bash

gdb  ./parser \
 -ex "break main"\
 -ex "run ../Test/test1.cmm"