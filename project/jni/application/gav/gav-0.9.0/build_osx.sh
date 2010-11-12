rm -rf gav.app
mkdir -p gav.app/Contents/MacOS
cp gav gav.app/Contents/MacOS
echo "APPL????" >gav.app/Contents/PkgInfo
cp osx-info.plist gav.app/Contents/Info.plist