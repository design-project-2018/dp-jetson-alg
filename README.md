# Design Project Preprocessor Algorithm

This repository holds the code for the machine learning algorithm preprecessor that will be used for real time
rear end detection. The compiled executable created from this project will be ran as a child process through
the Design Project Jetson Board Server.

## Prerequisites

The code in this project was designed to be run on a NVIDIA Jetson TX2 GPU Board. All the proper drivers required
for CUDA enabled development must be installed in order to run this algorithm at its highest performing speed.

## Setting Up Development Environment

OpenCV, Darknet, CLang, and CMake must be installed in the development environment to properly build the project.
In addition, the CLion IDE is highly recommended as a development environment. Scripts have been written
to install these on a linux environment and can be ran using the following commands.

```
cd <PROJECT ROOT DIRECTORY>/provision
chmod u+x install_compiler.sh
chmod u+x install_opencv.sh
chmod u+x install_clion.sh
chmod u+x install_darknet.sh
./install_compiler.sh
./install_opencv.sh
./install_clion.sh
./install_darknet.sh
```

Once installed, you can open up CLion by entering the follow command.

```
/opt/clion-2016.2/bin/clion.sh
```

## Running Main Preprocessor

In order to run our project, we must compile it using cmake which will build an executable file.
As of now the executable takes in a sample video input which is put in as a command line argument.
You can run the project with the following commands:

```
cd <PROJECT ROOT DIRECTORY>
mkdir build && cd build
cmake .. && make
cd ..
./preprocessor <PATH TO VIDEO FOOTAGE>
```

## Running Object Extractor

The object extractor function, takes in an input footage and performs yolo detection on each frame.
The result is then outputted to a json file that will be used train the model.

The object extractor can be run with the following commands:

```
cd <PROJECT ROOT DIRECTORY>
mkdir build && cd build
cmake .. && make
cd ..
./object_extractor <PATH TO INPUT VIDEO> <OUTPUT FOLDER>
```
