VERSION=stable
rm -rf LICENSE.md src
curl -L -O https://github.com/sansumbrella/Choreograph/archive/refs/heads/$VERSION.zip
mkdir .choreograph && bsdtar --strip-components=1 -xvf *.zip  -C .choreograph

cp .choreograph/LICENSE.md .
cp -r .choreograph/src/choreograph src

rm -rf *.zip .choreograph
