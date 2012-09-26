#!/bin/bash

SED=sed

if [[ "`uname`" == "Darwin" ]]; then
  SED=gsed
fi

# Rename modules to lowercase:
# import "mvp/Frontend/SessionDesc.proto";
# becomes
# import "mvp/frontend/SessionDesc.proto";
$SED -e 's/^\(\s*import\s*"\)\(.*\)\(\/.*\)/\1\L\2\E\3/' $1 > $2
