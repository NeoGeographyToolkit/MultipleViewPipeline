#!/bin/sh

if [[ "$#" != "1" ]]; then
  echo "Usage: $0 VW_SRC_DIR"
  exit
fi

SCRIPT_DIR=$(dirname $0)
PROTO_DIR=$SCRIPT_DIR/proto

VW_SRC_DIR="$1"/src/vw

if [[ ! -d "$VW_SRC_DIR" ]]; then
  echo "Not a valid VW root src dir: $VW_SRC_DIR"
  exit
fi

rsync -atv --prune-empty-dirs --include "*/" --include "*.proto" --exclude '*' $VW_SRC_DIR $SCRIPT_DIR
