/*=========================================================================
 *
 *  Copyright Insight Software Consortium
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
#ifndef __itkTestingComparisonImageFilter_hxx
#define __itkTestingComparisonImageFilter_hxx

#include "itkTestingComparisonImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkProgressReporter.h"

namespace itk
{
namespace Testing
{
// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
ComparisonImageFilter< TInputImage, TOutputImage >::ComparisonImageFilter()
{
  this->DynamicMultiThreadingOff(); // NEEDED FOR ITKv5 backwards compatibility
  // We require two inputs to execute.
  this->SetNumberOfRequiredInputs( 2 );

  // Set the default DifferenceThreshold.
  m_DifferenceThreshold = NumericTraits< OutputPixelType >::ZeroValue();

  // Set the default ToleranceRadius.
  m_ToleranceRadius = 0;

  // Initialize statistics about difference image.
  m_MeanDifference = NumericTraits< RealType >::ZeroValue();
  m_TotalDifference = NumericTraits< AccumulateType >::ZeroValue();
  m_NumberOfPixelsWithDifferences = 0;
  m_IgnoreBoundaryPixels = false;
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::PrintSelf( std::ostream & os, Indent indent ) const
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "ToleranceRadius: " << m_ToleranceRadius << "\n";
  os << indent << "DifferenceThreshold: " << m_DifferenceThreshold << "\n";
  os << indent << "MeanDifference: " << m_MeanDifference << "\n";
  os << indent << "TotalDifference: " << m_TotalDifference << "\n";
  os << indent << "NumberOfPixelsWithDifferences: " << m_NumberOfPixelsWithDifferences << "\n";
  os << indent << "IgnoreBoundaryPixels: " << m_IgnoreBoundaryPixels << "\n";
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::SetValidInput( const InputImageType * validImage )
{
  // The valid image should be input 0.
  this->SetInput( 0, validImage );
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::SetTestInput( const InputImageType * testImage )
{
  // The test image should be input 1.
  this->SetInput( 1, testImage );
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::BeforeThreadedGenerateData()
{
  ThreadIdType numberOfThreads = this->GetNumberOfThreads();

  // Initialize statistics about difference image.
  m_MeanDifference = NumericTraits< RealType >::ZeroValue();
  m_TotalDifference = NumericTraits< AccumulateType >::ZeroValue();
  m_NumberOfPixelsWithDifferences = 0;

  // Resize the thread temporaries
  m_ThreadDifferenceSum.SetSize( numberOfThreads );
  m_ThreadNumberOfPixels.SetSize( numberOfThreads );

  // Initialize the temporaries
  m_ThreadDifferenceSum.Fill( NumericTraits< AccumulateType >::ZeroValue() );
  m_ThreadNumberOfPixels.Fill( 0 );
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::ThreadedGenerateData( const OutputImageRegionType & threadRegion,
                                                                          int                           threadId )
{
  using SmartIterator = ConstNeighborhoodIterator< InputImageType >;
  using InputIterator = ImageRegionConstIterator< InputImageType >;
  using OutputIterator = ImageRegionIterator< OutputImageType >;
  using FacesCalculator = NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType >;
  using RadiusType = typename FacesCalculator::RadiusType;
  using FaceListType = typename FacesCalculator::FaceListType;
  using FaceListIterator = typename FaceListType::iterator;
  using InputPixelType = typename InputImageType::PixelType;

  // Prepare standard boundary condition.
  ZeroFluxNeumannBoundaryCondition< InputImageType > nbc;

  // Get a pointer to each image.
  const InputImageType * validImage = this->GetInput( 0 );
  const InputImageType * testImage = this->GetInput( 1 );
  OutputImageType *      outputPtr = this->GetOutput();

  if ( validImage->GetBufferedRegion() != testImage->GetBufferedRegion() )
  {
    itkExceptionMacro( << "Input images have different Buffered Regions." )
  }

  // Create a radius of pixels.
  RadiusType                           radius;
  const unsigned int                   minVoxelsNeeded = m_ToleranceRadius * 2 + 1;
  const typename TInputImage::SizeType imageSize = validImage->GetBufferedRegion().GetSize();
  for ( unsigned int d = 0; d < TInputImage::ImageDimension; ++d )
  {
    if ( minVoxelsNeeded < imageSize[d] )
    {
      radius[d] = m_ToleranceRadius;
    }
    else
    {
      radius[d] = ( ( imageSize[d] - 1 ) / 2 );
    }
  }

  // Find the data-set boundary faces.
  FacesCalculator boundaryCalculator;
  FaceListType    faceList = boundaryCalculator( testImage, threadRegion, radius );

  // Support progress methods/callbacks.
  ProgressReporter progress( this, threadId, threadRegion.GetNumberOfPixels() );
  // Process the internal face and each of the boundary faces.
  for ( FaceListIterator face = faceList.begin(); face != faceList.end(); ++face )
  {
    SmartIterator  test( radius, testImage, *face ); // Iterate over test image.
    InputIterator  valid( validImage, *face );       // Iterate over valid image.
    OutputIterator out( outputPtr, *face );          // Iterate over output image.
    if ( !test.GetNeedToUseBoundaryCondition() || !m_IgnoreBoundaryPixels )
    {
      test.OverrideBoundaryCondition( &nbc );
      for ( valid.GoToBegin(), test.GoToBegin(), out.GoToBegin(); !valid.IsAtEnd(); ++valid, ++test, ++out )
      {
        // Get the current valid pixel.
        InputPixelType t = valid.Get();

        //  Assume a good match - so test center pixel first, for speed
        RealType difference = static_cast< RealType >( t ) - test.GetCenterPixel();
        if ( NumericTraits< RealType >::IsNegative( difference ) )
        {
          difference = -difference;
        }
        OutputPixelType minimumDifference = static_cast< OutputPixelType >( difference );

        // If center pixel isn't good enough, then test the neighborhood
        if ( minimumDifference > m_DifferenceThreshold )
        {
          unsigned int neighborhoodSize = test.Size();
          // Find the closest-valued pixel in the neighborhood of the test
          // image.
          for ( unsigned int i = 0; i < neighborhoodSize; ++i )
          {
            // Use the RealType for the difference to make sure we get the
            // sign.
            RealType differenceReal = static_cast< RealType >( t ) - test.GetPixel( i );
            if ( NumericTraits< RealType >::IsNegative( differenceReal ) )
            {
              differenceReal = -differenceReal;
            }
            OutputPixelType d = static_cast< OutputPixelType >( differenceReal );
            if ( d < minimumDifference )
            {
              minimumDifference = d;
              if ( minimumDifference <= m_DifferenceThreshold )
              {
                break;
              }
            }
          }
        }

        // Check if difference is above threshold.
        if ( minimumDifference > m_DifferenceThreshold )
        {
          // Store the minimum difference value in the output image.
          out.Set( minimumDifference );

          // Update difference image statistics.
          m_ThreadDifferenceSum[threadId] += minimumDifference;
          m_ThreadNumberOfPixels[threadId]++;
        }
        else
        {
          // Difference is below threshold.
          out.Set( NumericTraits< OutputPixelType >::ZeroValue() );
        }

        // Update progress.
        progress.CompletedPixel();
      }
    }
    else
    {
      for ( out.GoToBegin(); !out.IsAtEnd(); ++out )
      {
        out.Set( NumericTraits< OutputPixelType >::ZeroValue() );
        progress.CompletedPixel();
      }
    }
  }
}

// ----------------------------------------------------------------------------
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::AfterThreadedGenerateData()
{
  // Set statistics about difference image.
  ThreadIdType numberOfThreads = this->GetNumberOfThreads();

  for ( ThreadIdType i = 0; i < numberOfThreads; ++i )
  {
    m_TotalDifference += m_ThreadDifferenceSum[i];
    m_NumberOfPixelsWithDifferences += m_ThreadNumberOfPixels[i];
  }

  // Get the total number of pixels processed in the region.
  // This is different from the m_TotalNumberOfPixels which
  // is the number of pixels that actually have differences
  // above the intensity threshold.
  OutputImageRegionType region = this->GetOutput()->GetRequestedRegion();
  AccumulateType        numberOfPixels = region.GetNumberOfPixels();

  // Calculate the mean difference.
  m_MeanDifference = m_TotalDifference / numberOfPixels;
}

/**
 *
 */
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::SetInput( const TInputImage * input )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 0, const_cast< TInputImage * >( input ) );
}

/**
 * Connect one of the operands for pixel-wise addition
 */
template < typename TInputImage, typename TOutputImage >
void
ComparisonImageFilter< TInputImage, TOutputImage >::SetInput( unsigned int index, const TInputImage * image )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( index, const_cast< TInputImage * >( image ) );
}

/**
 *
 */
template < typename TInputImage, typename TOutputImage >
const TInputImage *
ComparisonImageFilter< TInputImage, TOutputImage >::GetInput( void ) const
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return 0;
  }

  return static_cast< const TInputImage * >( this->ProcessObject::GetInput( 0 ) );
}

/**
 *
 */
template < typename TInputImage, typename TOutputImage >
const TInputImage *
ComparisonImageFilter< TInputImage, TOutputImage >::GetInput( unsigned int idx ) const
{
  return static_cast< const TInputImage * >( this->ProcessObject::GetInput( idx ) );
}
} // end namespace Testing
} // end namespace itk

#endif
