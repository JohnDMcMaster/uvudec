#!/bin/bash
git pull
export LD_LIBRARY_PATH=$PWD/uvudec/uvudec/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PWD/uvudec/uvudec/lib/plugin:$LD_LIBRARY_PATH
python uvbuilder.py $@
