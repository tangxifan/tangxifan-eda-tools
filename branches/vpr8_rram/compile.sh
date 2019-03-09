rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-8 -DCMAKE_C_COMPILER=gcc-8
make -j8
