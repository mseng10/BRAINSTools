The BRAINSTools project is a harness to assist in building the many BRAINSTools under development.

# Building
Example session for a clean build on Linux using gcc-4.2:
```bash
DIRECTORY=~/src/mytester
mkdir ${DIRECTORY}
cd ${DIRECTORY}
git clone git://github.com/BRAINSia/BRAINSTools.git
```
__For developers:__

```bash
cd ${DIRECTORY}/BRAINSTools/
bash ./Utilities/SetupForDevelopment.sh
```
###Mac OSX
Building BRAINSTools on mac is the same as making any standard out of source cmake build.

__1.__ Clone BRAINSTools into a directory:
```bash
git clone https://github.com/BRAINSTools.git BRAINSTools
```
__2.__ Run the set-up script:
```bash
cd BRAINSTools
./Utilities/SetupForDevelopment.sh
cd ..
```
__3.__ Create out of source build directory:
```bash
mkdir BRAINSTools-build
cd BRIANSTools-build
cmake ../BRAINSTools

## NOTE: To configure in cmake gui
##    ccmake ../BRAINSTools

make -j${NUMOFTHREADS} -k

## NOTE: To find the number of threads from the OSX terminal, use:
##    sysctl -n hw.ncpu
```

###Linux RedHat 6
Example of clean build on Linus RedHat 6.

__1.__ Clone BRAINSTools into a directory:
```bash
mkdir ~/src/mytester
cd ~/src/mytester
git clone https://github.com/BRAINSTools.git
``` 
__2.__ Run the set-up script:
```bash
cd BRAINSTools/
bash ./Utilities/SetupForDevelopment.sh
``` 
__3.__ Create out of source build directory:
```bash
mkdir -p ../BRAINSTools-build
cd ../BRAINSTools-build/
CC=/usr/bin/gcc-4.2 CXX=/usr/bin/g++-4.2 ccmake ../BRAINSTools \
    -DUSE_BRAINSConstellationDetector:BOOL=ON \
    -DUSE_BRAINSABC:BOOL=ON

make -j${NUMOFTHREADS} -k

## NOTE: The fetching of data still has problems with parallel builds, so we need to restart it at least once
make
``` 

###Linux Debian (Ubuntu)
Building BRAINSTools on a fresh install has the additional dependency of building CMake on your system. You cannot use the version 
from apt-get as that does some unnatural things with Python resources to be backwards compatible 
(see http://public.kitware.com/Bug/view.php?id=14156).

__1.__ Install dependencies:
```bash
udo apt-get update
sudo apt-get upgrade
sudo apt-get install git python2.7
python2.7-dev g++ freeglut3-dev
```
__2.__ Get CMake binaries:
```bash
# You can find the URL of the latest CMake binary by
# * Go to www.cmake.org/download in a browser
# * Hover over the desired 'Binary distribution' for your operating
#     system
# * Right-click and select 'Copy Link Address' for the file ending in
#     "tar.gz"
URL=http://www.cmake.org/files/v3.2/cmake-3.2.1-Linux-x86_64.tar.gz
wget ${URL}
# Decompress the file
tar -xzvf cmake-3.2.1-Linux-x86_64.tar.gz
# Add the binary to your PATH environment variable
export PATH=${PWD}/cmake-3.2.1-Linux-x86_64/bin:${PATH}
```
__3.__ Clone the repository and build:
```bash
git clone https://github.com/BRAINSTools/BRAINSTools.git
mkdir BRAINSTools-build
cd BRAINSTools-build
CC=/usr/bin/gcc-4.8 \
CXX=/usr/bin/g++-4.8 \
cmake ../BRAINSTools \
make -j${NUMOFTHREADS} -k

##You can find hte number of threads with:
##    lscpu
```

# Development
Developers should run the "./Utilities/SetupForDevelopment.sh" script to get started.

# Testing
`BRAINSTools_MAX_TEST_LEVEL` adjusts how agressive the test suite is so that long running tests or incomplete tests can easily be silenced

```cmake
set(BRAINSTools_MAX_TEST_LEVEL 3 
      CACHE STRING "Testing level for managing test burden")
```

__1__ - Run the absolute minimum (very fast tests) 
  * These should always pass before any code commit!

__3__ - Run fast tests on continous builds.
* These need immediate attention if they begin to fail!

__5__ - Run moderate nightly tests.
  * These need immediate attention if they begin to fail!

__7__ - Run long running extensive test that are a burden to normal development.
  * Testing done 1x per week.

__8__ - Run tests that fail due to incomplete test building. 
  * These are good ideas for test that we don't have time to make robust.

__9__ - Run tests that don't have much utility currently.

***

###### Example
setting a test's max level in TestSuite/CMakeLists.txt
```cmake
if( ${BRAINSTools_MAX_TEST_LEVEL} GREATER 8)
  ExternalData_add_test(FindCenterOfBrainFetchData
    NAME itkResampleInPlaceImageFilterTest
    COMMAND $<TARGET_FILE:itkResampleInPlaceImageFilterTest>
      itkResampleInPlaceImageFilterTest input1 transform1 checkresult
  )
```
