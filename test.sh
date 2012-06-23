#!/bin/sh

train-kytea -full test/data/test.txt -model test/data/test.mod > /dev/null 2&>1
node test/test.js

if [ $? -eq 0 ]; then
	echo "SUCCESS"
	exit 0
else
	echo "FAILED"
	exit 1
fi
