export LD_RUN_PATH=lib/osx_x64/
make all -j4 -f Makefile.osx
install_name_tool -change /usr/local/lib/libirrklang.dylib @executable_path/lib/osx_x64/libirrklang.dylib drive64
