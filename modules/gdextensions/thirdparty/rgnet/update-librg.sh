VERSION=7.3.0
rm -rf AUTHORS inc LICENSE src *.zip
curl -L -O https://github.com/zpl-c/librg/archive/v$VERSION.zip
mkdir .librg && bsdtar --strip-components=1 -xvf *.zip  -C .librg

rm -rf header vendor *.h

cp -r .librg/code/header .
cp -r .librg/code/vendor .
cp .librg/code/librg.h .librg/code/librg_hedley.h .

rm -rf *.zip .librg
