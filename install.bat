del /q/f nevk.exe
mkdir build
cd build
cmake ..
cmake --build .
cd Debug
copy /B nevk.exe ..\..\nevk.exe
cd ..\..\
rd /s/q build
