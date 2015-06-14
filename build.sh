set -e

mkdir -p build/animation

mkdir -p build/raytracer.app/Contents/MacOS
mkdir -p build/raytracer.app/Contents/Resources

clang++ -dynamiclib src/app.cpp -o build/raytracer.app/Contents/Resources/app.dylib -g -O2 -ffast-math
clang++ src/osx_main.cpp -o build/raytracer.app/Contents/MacOS/raytracer -lSDL2 -g -O2 -framework OpenGl
