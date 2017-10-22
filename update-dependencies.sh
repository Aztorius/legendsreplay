#!/bin/sh

echo "preparing the latest version of 3rd-party dependencies ...\r\n"

cd qhttp
mkdir 3rdparty
cd 3rdparty

echo "--> nodejs / http-parser"
if [ -e http-parser ]; then
    cd http-parser
    git pull origin master
else
    git clone https://github.com/nodejs/http-parser.git
fi

cd ..
cd ..
echo "\r\ndone."
