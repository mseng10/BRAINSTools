/*=========================================================================
 *
 *  Copyright SINAPSE: Scalable Informatics for Neuroscience, Processing and Software Engineering
 *            The University of Iowa
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
//
// A test driver to append the
// itk image processing test
// commands to an
// the SEM compatibile program
//

#ifdef WIN32
#  define MODULE_IMPORT __declspec( dllimport )
#else
#  define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int
ModuleEntryPoint( int, char *[] );

int
BRAINSMultiSTAPLETest( int argc, char * argv[] )
{
  return ModuleEntryPoint( argc, argv );
}
