The BRAINSTools project is a harness to assist in building the many BRAINSTools under development.

## Building
Example sessions for a clean build.

### Mac OSX
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

## Development
Developers should follow these instructions:
```bash
cd ${DIRECTORY}/BRAINSTools/
bash ./Utilities/SetupForDevelopment.sh
```
If developing on Mac OSX, make sure the xcode command line tools are installed with the command:
```bash
xcode-select --install
``` 

## Testing
`BRAINSTools_MAX_TEST_LEVEL` adjusts how aggressive the test suite is so that long running tests or incomplete tests can easily be silenced

```cmake
set(BRAINSTools_MAX_TEST_LEVEL 3 
      CACHE STRING "Testing level for managing test burden")
```

__1__ - Run the absolute minimum (very fast tests) 
  * These should always pass before any code commit!

__2__ - Run fast tests on continuous builds.
* These need immediate attention if they begin to fail!

__3__ - Run moderate nightly tests.
  * These need immediate attention if they begin to fail!

__4__ - Run long running extensive test that are a burden to normal development.
  * Testing done 1x per week.

__5__ - Run tests that fail due to incomplete test building. 
  * These are good ideas for test that we don't have time to make robust.

__6__ - Run tests that don't have much utility currently.

***

##### Example
setting a test's max level in TestSuite/CMakeLists.txt
```cmake
if( ${BRAINSTools_MAX_TEST_LEVEL} GREATER 8)
  ExternalData_add_test(FindCenterOfBrainFetchData
    NAME itkResampleInPlaceImageFilterTest
    COMMAND $<TARGET_FILE:itkResampleInPlaceImageFilterTest>
      itkResampleInPlaceImageFilterTest input1 transform1 checkresult
  )
```
## External Links
__1.__ More information on individual [BRAINSTools](/https://github.com/BRAINSia/BRAINSTools/wiki)

__2.__ NAMICExternalProjects providing set of [tools](/https://github.com/BRAINSia/NAMICExternalProjects)

__3.__ [ANTs](/https://github.com/ANTsX/ANTs) package
