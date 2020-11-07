mkdir build
cd build
cmake ..
cmake --build .
cd Debug
copy /B myopengl.exe ..\..\myopengl.exe
cd ..\..\
rd /s/q build
