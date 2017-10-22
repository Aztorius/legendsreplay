echo "preparing the latest version of 3rd-party dependencies ..."

cd qhttp
mkdir 3rdparty
cd 3rdparty

echo "--> nodejs / http-parser"
git clone https://github.com/nodejs/http-parser.git

cd ..
cd ..
echo "done."
