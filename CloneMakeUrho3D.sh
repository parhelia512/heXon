#!/bin/sh
sudo apt-get install libx11-dev libxrandr-dev libasound2-dev git cmake make

if [ ! -d Urho3D ]
then
  git clone https://github.com/Urho3D/Urho3D
fi

cd Urho3D 
cmake .
make
cd ..