set -e

mkdir -p build
mkdir -p build/animation

clang++ -dynamiclib app.cpp -o build/app.dylib -g -O2 -ffast-math
clang++ osx_main.cpp -o build/main -lSDL2 -g -O2 -framework OpenGl
