# Design Project Jetson Board

This repository holds the code for the program that will be installed onto our Jetson Board. This project will hold
the core logic for all computer vision computations in the overall design project and will connect to the camera,
mobile application, and the jetson board server.

## Prerequisites

Due to the large number of dependencies, all development is recommended to be done within a vagrant virtual
environment. Before setting up the project, make sure to install [Vagrant](https://www.vagrantup.com/) and
[Virtual Box](https://www.virtualbox.org/).

## Setting Up Virtual Environment

To setup the virtual environment, we can simply the following lines of code.

```
cd <PROJECT ROOT DIRECTORY>
vagrant up
```

This will start up our VM and build all the dependencies by following the bash script in the provision folder. Any
dependencies added during development should be included in the script.

## Developing In The Virtual Environment

After starting up the virtual environment, a GUI should pop up for our VM with a login screen. The user and password
are "vagrant". Once logged in, you can open up CLion by opening a terminal and entering the follow commands.

```
cd <PROJECT ROOT DIRECTORY>
/opt/clion-2016.2/bin/clion.sh
```

This should open up the CLion IDE with all the dependencies configured.

## Running Project

In order to run our project, we must compile and execute it using cmake (inside the virtual environment)

```
cd <PROJECT ROOT DIRECTORY>
mkdir build && cd build
cmake ..
make
./dp_jetson_board
```

## Shutting Down Virtual Environment

When not developing, be sure to shut down your virtual environment so the memory can be freed up on the computer for
other tasks. To shut down the virtual environment, enter the follow commands in terminal (outside of the virtual env).

```
cd <PROJECT ROOT DIRECTORY>
vagrant halt
```

## Destroying The Virtual Environment

In the case you would like to destroy the virtual environment, follow the following commands (outside of the VM).

```
cd <PROJECT ROOT DIRECTORY>
vagrant destroy
```

## Deploying The Project

TBD