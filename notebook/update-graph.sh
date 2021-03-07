#!/bin/bash

set -euxo pipefail

TMP_DIR=/tmp/jupyter/airquality
TARGET_DIR="/var/www/html/airquality/"
rm -rf "$TMP_DIR"
cd $HOME/dev/airquality-sender/notebook
make run
rsync -avh "$TMP_DIR/" "$TARGET_DIR"
cp index.html "$TARGET_DIR"
