# Windows Builds

Not recommended. Windows sucks!

`cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja --preset windows-msvc-x64`

# Linux Builds

Install dependencies:

`sudo apt install -y libsfml-dev libudev-dev libopenal-dev libvorbis-dev libflac-dev libxrandr-dev libxcursor-dev libfreetype-dev`

And maybe also libfreetype6 and libfreetype6-dev, not sure...
