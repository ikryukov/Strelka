del /q/f oka.exe
mkdir build
cd build
cmake ..
cmake --build .
cd Debug
copy /B oka.exe ..\..\oka.exe
cd ..\..\
rd /s/q build
