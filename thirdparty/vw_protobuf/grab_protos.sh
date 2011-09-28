#!/bin/sh

if [[ "$#" != "1" ]]; then
  echo "Usage: $0 VW_SRC_DIR"
fi

SCRIPT_DIR=$(dirname $0)
PROTO_DIR=$SCRIPT_DIR/proto

VW_SRC_DIR="$1"

#rsync -atv --prune-empty-dirs --include "*/" --include "*.proto" --exclude '*' $VW_SRC_DIR $SCRIPT_DIR

mkdir -p $PROTO_DIR
find $VW_SRC_DIR -iname '*.proto' -print -exec cp {} $PROTO_DIR \;
