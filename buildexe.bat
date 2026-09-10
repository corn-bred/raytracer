@echo off
FOR %%A IN (%*) DO (
    IF "%%A"=="--static" ECHO Creating statically linked build
)
FOR %%A IN (%*) DO (
    IF "%%A"=="--run" (
      ECHO Building and running...
    )
)

FOR %%A IN (%*) DO (
    IF "%%A"=="--static" (
      cmake -S . -B build -G Ninja -DSTATIC_BUILD=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
    ) ELSE (
      cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
    )
)
cmake --build build

IF "%1"=="--run" (
    .\bin\Raytracing.exe
)