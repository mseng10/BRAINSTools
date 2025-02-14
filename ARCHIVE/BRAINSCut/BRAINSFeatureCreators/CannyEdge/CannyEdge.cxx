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
#if defined( _MSC_VER )
#  pragma warning( disable : 4786 )
#endif

#ifdef __BORLANDC__
#  define ITK_LEAN_AND_MEAN
#endif

//  Software Guide : BeginLatex
//
//  This example introduces the use of the
//  \doxygen{CannyEdgeDetectionImageFilter}. This filter is widely used for
//  edge detection since it is the optimal solution satisfying the constraints
//  of good sensitivity, localization and noise robustness.
//
//  \index{itk::CannyEdgeDetectionImageFilter|textbf}
//
//  Software Guide : EndLatex

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

//  Software Guide : BeginLatex
//
//  The first step required for using this filter is to include its header file
//
//  \index{itk::CannyEdgeDetectionImageFilter!header}
//
//  Software Guide : EndLatex

// Software Guide : BeginCodeSnippet
#include "itkCannyEdgeDetectionImageFilter.h"
// Software Guide : EndCodeSnippet

#include "CannyEdgeCLP.h"

int
main( int argc, char * argv[] )
{
  PARSE_ARGS;
  // float variance = 2.0;
  // float upperThreshold = 0.0;
  // float lowerThreshold = 0.0;
  std::cout << "Variance = " << variance << std::endl;
  std::cout << "UpperThreshold = " << upperThreshold << std::endl;
  std::cout << "LowerThreshold = " << lowerThreshold << std::endl;

  using CharPixelType = float; //  IO
  using RealPixelType = float; //  Operations
  constexpr unsigned int Dimension = 3;

  using CharImageType = itk::Image< CharPixelType, Dimension >;
  using RealImageType = itk::Image< RealPixelType, Dimension >;

  using ReaderType = itk::ImageFileReader< CharImageType >;
  using WriterType = itk::ImageFileWriter< CharImageType >;

  //  Software Guide : BeginLatex
  //  This filter operates on image of pixel type float. It is then necessary
  //  to cast the type of the input images that are usually of integer type.
  //  The \doxygen{CastImageFilter} is used here for that purpose. Its image
  //  template parameters are defined for casting from the input type to the
  //  float type using for processing.
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  using CastToRealFilterType = itk::CastImageFilter< CharImageType, RealImageType >;
  // Software Guide : EndCodeSnippet

  using RescaleFilter = itk::RescaleIntensityImageFilter< RealImageType, CharImageType >;

  //  Software Guide : BeginLatex
  //  The \doxygen{CannyEdgeDetectionImageFilter} is instantiated using the
  // float image type.
  //  \index{itk::CannyEdgeDetectionImageFilter|textbf}
  //  Software Guide : EndLatex

  using CannyFilter = itk::CannyEdgeDetectionImageFilter< RealImageType, RealImageType >;

  // Setting the IO

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
  RescaleFilter::Pointer        rescale = RescaleFilter::New();

  // Setting the ITK pipeline filter

  CannyFilter::Pointer cannyFilter = CannyFilter::New();

  reader->SetFileName( inputVolume );
  writer->SetFileName( outputVolume );

  // The output of an edge filter is 0 or 1
  rescale->SetOutputMinimum( 0 );
  rescale->SetOutputMaximum( 255 );

  toReal->SetInput( reader->GetOutput() );

  cannyFilter->SetInput( toReal->GetOutput() );
  cannyFilter->SetVariance( variance );
  cannyFilter->SetUpperThreshold( upperThreshold );
  cannyFilter->SetLowerThreshold( lowerThreshold );

  rescale->SetInput( cannyFilter->GetOutput() );
  writer->SetInput( rescale->GetOutput() );
  writer->UseCompressionOn();

  try
  {
    writer->Update();
  }
  catch ( itk::ExceptionObject & err )
  {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
