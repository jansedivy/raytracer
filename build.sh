set -e

clang++ -dynamiclib app.cpp -o build/app.dylib -g
clang++ osx_main.cpp -o build/main -lSDL2 -g -O2 -framework OpenGl
