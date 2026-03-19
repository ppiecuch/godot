VERSION=0.9
rm -rf LICENSE src *.zip
curl -L -O https://github.com/litehtml/litehtml/archive/v$VERSION.zip
bsdtar --strip-components=1 -xvf *.zip
rm *.zip
rm -rf CMakeLists.txt docs cmake containers test .git* *.md *.txt *.vcxproj*
