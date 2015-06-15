set -e

mkdir -p build/animation

mkdir -p build/raytracer.app/Contents/MacOS
mkdir -p build/raytracer.app/Contents/Resources

clang++ -dynamiclib src/app.cpp -o build/raytracer.app/Contents/Resources/app.dylib -g -O2 -ffast-math
clang++ src/osx_main.cpp -o build/raytracer.app/Contents/MacOS/raytracer -I./build/raytracer.app/Contents/Frameworks/Headers -F ./build/raytracer.app/Contents/Frameworks -framework SDL2 -O2 -framework OpenGl -rpath @executable_path/../Frameworks
