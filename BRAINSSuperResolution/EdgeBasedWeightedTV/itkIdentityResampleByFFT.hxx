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
#ifndef itkResampleImageFilter_hxx
#define itkResampleImageFilter_hxx

#include "itkResampleImageFilter.h"
#include "itkObjectFactory.h"
#include "itkIdentityTransform.h"
#include "itkProgressReporter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageScanlineIterator.h"
#include "itkSpecialCoordinatesImage.h"
#include "itkDefaultConvertPixelTraits.h"

namespace itk
{
/**
 * Initialize new instance
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::ResampleImageFilter()
{
  m_OutputOrigin.Fill( 0.0 );
  m_OutputSpacing.Fill( 1.0 );
  m_OutputDirection.SetIdentity();

  m_UseReferenceImage = false;

  m_Size.Fill( 0 );
  m_OutputStartIndex.Fill( 0 );

  // Pipeline input configuration

  // implicit:
  // #0 "Primary" required

  //  #1 "ReferenceImage" optional
  Self::AddRequiredInputName( "ReferenceImage", 1 );
  Self::RemoveRequiredInputName( "ReferenceImage" );

  //   "Transform" required ( not numbered )
  Self::AddRequiredInputName( "Transform" );
  Self::SetTransform( IdentityTransform< TTransformPrecisionType, ImageDimension >::New() );

  m_Interpolator = dynamic_cast< InterpolatorType * >( LinearInterpolatorType::New().GetPointer() );

  m_Extrapolator = nullptr;

  m_DefaultPixelValue = NumericTraits< PixelType >::ZeroValue( m_DefaultPixelValue );
}

/**
 * Print out a description of self
 *
 * \todo Add details about this class
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::PrintSelf(
  std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent
     << "DefaultPixelValue: " << static_cast< typename NumericTraits< PixelType >::PrintType >( m_DefaultPixelValue )
     << std::endl;
  os << indent << "Size: " << m_Size << std::endl;
  os << indent << "OutputStartIndex: " << m_OutputStartIndex << std::endl;
  os << indent << "OutputSpacing: " << m_OutputSpacing << std::endl;
  os << indent << "OutputOrigin: " << m_OutputOrigin << std::endl;
  os << indent << "OutputDirection: " << m_OutputDirection << std::endl;
  os << indent << "Transform: " << this->GetTransform() << std::endl;
  os << indent << "Interpolator: " << m_Interpolator.GetPointer() << std::endl;
  os << indent << "Extrapolator: " << m_Extrapolator.GetPointer() << std::endl;
  os << indent << "UseReferenceImage: " << ( m_UseReferenceImage ? "On" : "Off" ) << std::endl;
}

/**
 * Set the output image spacing.
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::SetOutputSpacing(
  const double * spacing )
{
  SpacingType s;
  for ( unsigned int i = 0; i < TOutputImage::ImageDimension; ++i )
  {
    s[i] = static_cast< typename SpacingType::ValueType >( spacing[i] );
  }
  this->SetOutputSpacing( s );
}

/**
 * Set the output image origin.
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::SetOutputOrigin(
  const double * origin )
{
  OriginPointType p( origin );

  this->SetOutputOrigin( p );
}

/** Helper method to set the output parameters based on this image */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::SetOutputParametersFromImage( const ImageBaseType * image )
{
  this->SetOutputOrigin( image->GetOrigin() );
  this->SetOutputSpacing( image->GetSpacing() );
  this->SetOutputDirection( image->GetDirection() );
  this->SetOutputStartIndex( image->GetLargestPossibleRegion().GetIndex() );
  this->SetSize( image->GetLargestPossibleRegion().GetSize() );
}

/**
 * Set up state of filter before multi-threading.
 * InterpolatorType::SetInputImage is not thread-safe and hence
 * has to be set up before ThreadedGenerateData
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::BeforeThreadedGenerateData()
{

  if ( !m_Interpolator )
  {
    itkExceptionMacro( << "Interpolator not set" );
  }

  // Connect input image to interpolator
  m_Interpolator->SetInputImage( this->GetInput() );

  // Connect input image to extrapolator
  if ( !m_Extrapolator.IsNull() )
  {
    m_Extrapolator->SetInputImage( this->GetInput() );
  }

  unsigned int nComponents = DefaultConvertPixelTraits< PixelType >::GetNumberOfComponents( m_DefaultPixelValue );

  if ( nComponents == 0 )
  {
    PixelComponentType zeroComponent = NumericTraits< PixelComponentType >::ZeroValue( zeroComponent );
    nComponents = this->GetInput()->GetNumberOfComponentsPerPixel();
    NumericTraits< PixelType >::SetLength( m_DefaultPixelValue, nComponents );
    for ( unsigned int n = 0; n < nComponents; n++ )
    {
      PixelConvertType::SetNthComponent( n, m_DefaultPixelValue, zeroComponent );
    }
  }
}

/**
 * Set up state of filter after multi-threading.
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::AfterThreadedGenerateData()
{
  // Disconnect input image from the interpolator
  m_Interpolator->SetInputImage( nullptr );
  if ( !m_Extrapolator.IsNull() )
  {
    // Disconnect input image from the extrapolator
    m_Extrapolator->SetInputImage( nullptr );
  }
}

/**
 * ThreadedGenerateData
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::
  ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId )
{
  // Check whether the input or the output is a
  // SpecialCoordinatesImage.  If either are, then we cannot use the
  // fast path since index mapping will definitely not be linear.
  using OutputSpecialCoordinatesImageType = SpecialCoordinatesImage< PixelType, ImageDimension >;
  using InputSpecialCoordinatesImageType = SpecialCoordinatesImage< InputPixelType, InputImageDimension >;

  const bool isSpecialCoordinatesImage =
    ( dynamic_cast< const InputSpecialCoordinatesImageType * >( this->GetInput() ) ||
      dynamic_cast< const OutputSpecialCoordinatesImageType * >( this->GetOutput() ) );

  // Check whether we can use a fast path for resampling. Fast path
  // can be used if the transformation is linear. Transform respond
  // to the IsLinear() call.
  if ( !isSpecialCoordinatesImage && this->GetTransform()->GetTransformCategory() == TransformType::Linear )
  {
    this->LinearThreadedGenerateData( outputRegionForThread, threadId );
    return;
  }

  // Otherwise, we use the normal method where the transform is called
  // for computing the transformation of every point.
  this->NonlinearThreadedGenerateData( outputRegionForThread, threadId );
}

/**
 * Cast from interpolotor output to pixel type
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
typename ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                              TTransformPrecisionType >::PixelType
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::CastPixelWithBoundsChecking( const InterpolatorOutputType value,
                                                                             const ComponentType          minComponent,
                                                                             const ComponentType maxComponent ) const
{
  const unsigned int nComponents = InterpolatorConvertType::GetNumberOfComponents( value );
  PixelType          outputValue;

  NumericTraits< PixelType >::SetLength( outputValue, nComponents );

  for ( unsigned int n = 0; n < nComponents; n++ )
  {
    ComponentType component = InterpolatorConvertType::GetNthComponent( n, value );

    if ( component < minComponent )
    {
      PixelConvertType::SetNthComponent( n, outputValue, static_cast< PixelComponentType >( minComponent ) );
    }
    else if ( component > maxComponent )
    {
      PixelConvertType::SetNthComponent( n, outputValue, static_cast< PixelComponentType >( maxComponent ) );
    }
    else
    {
      PixelConvertType::SetNthComponent( n, outputValue, static_cast< PixelComponentType >( component ) );
    }
  }

  return outputValue;
}

/**
 * NonlinearThreadedGenerateData
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::
  NonlinearThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId )
{
  // Get the output pointers
  OutputImageType * outputPtr = this->GetOutput();

  // Get this input pointers
  const InputImageType * inputPtr = this->GetInput();

  // Honor the SpecialCoordinatesImage isInside value returned
  // by TransformPhysicalPointToContinuousIndex
  using InputSpecialCoordinatesImageType = SpecialCoordinatesImage< InputPixelType, InputImageDimension >;
  const bool isSpecialCoordinatesImage = dynamic_cast< const InputSpecialCoordinatesImageType * >( inputPtr );

  // Get the input transform
  const TransformType * transformPtr = this->GetTransform();


  // Create an iterator that will walk the output region for this thread.
  using OutputIterator = ImageRegionIteratorWithIndex< TOutputImage >;
  OutputIterator outIt( outputPtr, outputRegionForThread );

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint; // Coordinates of current output pixel
  PointType inputPoint;  // Coordinates of current input pixel

  ContinuousInputIndexType inputIndex;

  // Support for progress methods/callbacks
  ProgressReporter progress( this, threadId, outputRegionForThread.GetNumberOfPixels() );

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelComponentType minValue = NumericTraits< PixelComponentType >::NonpositiveMin();
  const PixelComponentType maxValue = NumericTraits< PixelComponentType >::max();

  using OutputType = typename InterpolatorType::OutputType;
  const ComponentType minOutputValue = static_cast< ComponentType >( minValue );
  const ComponentType maxOutputValue = static_cast< ComponentType >( maxValue );

  // Walk the output region
  outIt.GoToBegin();

  while ( !outIt.IsAtEnd() )
  {
    // Determine the index of the current output pixel
    outputPtr->TransformIndexToPhysicalPoint( outIt.GetIndex(), outputPoint );

    // Compute corresponding input pixel position
    inputPoint = transformPtr->TransformPoint( outputPoint );
    const bool isInsideInput = inputPtr->TransformPhysicalPointToContinuousIndex( inputPoint, inputIndex );

    PixelType  pixval;
    OutputType value;
    // Evaluate input at right position and copy to the output
    if ( m_Interpolator->IsInsideBuffer( inputIndex ) && ( !isSpecialCoordinatesImage || isInsideInput ) )
    {
      value = m_Interpolator->EvaluateAtContinuousIndex( inputIndex );
      pixval = this->CastPixelWithBoundsChecking( value, minOutputValue, maxOutputValue );
      outIt.Set( pixval );
    }
    else
    {
      if ( m_Extrapolator.IsNull() )
      {
        outIt.Set( m_DefaultPixelValue ); // default background value
      }
      else
      {
        value = m_Extrapolator->EvaluateAtContinuousIndex( inputIndex );
        pixval = this->CastPixelWithBoundsChecking( value, minOutputValue, maxOutputValue );
        outIt.Set( pixval );
      }
    }

    progress.CompletedPixel();
    ++outIt;
  }
}


/**
 * LinearThreadedGenerateData
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::
  LinearThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId )
{
  // Get the output pointers
  OutputImageType * outputPtr = this->GetOutput();

  // Get this input pointers
  const InputImageType * inputPtr = this->GetInput();

  // Get the input transform
  const TransformType * transformPtr = this->GetTransform();

  // Create an iterator that will walk the output region for this thread.
  using OutputIterator = ImageScanlineIterator< TOutputImage >;

  OutputIterator outIt( outputPtr, outputRegionForThread );

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint; // Coordinates of current output pixel
  PointType inputPoint;  // Coordinates of current input pixel
  PointType tmpOutputPoint;
  PointType tmpInputPoint;

  ContinuousInputIndexType inputIndex;
  ContinuousInputIndexType tmpInputIndex;

  using VectorType = typename PointType::VectorType;
  VectorType delta; // delta in input continuous index coordinate frame

  IndexType index;

  const typename OutputImageRegionType::SizeType & regionSize = outputRegionForThread.GetSize();
  const SizeValueType numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / regionSize[0];

  // Support for progress methods/callbacks
  ProgressReporter progress( this, threadId, numberOfLinesToProcess );

  using OutputType = typename InterpolatorType::OutputType;

  // Cache information from the superclass
  PixelType defaultValue = this->GetDefaultPixelValue();

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelComponentType minValue = NumericTraits< PixelComponentType >::NonpositiveMin();
  const PixelComponentType maxValue = NumericTraits< PixelComponentType >::max();

  using OutputType = typename InterpolatorType::OutputType;
  const ComponentType minOutputValue = static_cast< ComponentType >( minValue );
  const ComponentType maxOutputValue = static_cast< ComponentType >( maxValue );

  // Determine the position of the first pixel in the scanline
  index = outIt.GetIndex();
  outputPtr->TransformIndexToPhysicalPoint( index, outputPoint );

  // Compute corresponding input pixel position
  inputPoint = transformPtr->TransformPoint( outputPoint );
  inputPtr->TransformPhysicalPointToContinuousIndex( inputPoint, inputIndex );

  // As we walk across a scan line in the output image, we trace
  // an oriented/scaled/translated line in the input image.  Cache
  // the delta along this line in continuous index space of the input
  // image. This allows us to use vector addition to model the
  // transformation.
  //
  // To find delta, we take two pixels adjacent in a scanline
  // and determine the continuous indices of these pixels when
  // mapped to the input coordinate frame. We use the difference
  // between these two continuous indices as the delta to apply
  // to an index to trace line in the input image as we move
  // across the scanline of the output image.
  //
  // We determine delta in this manner so that Images
  // are both handled properly (taking into account the direction cosines).
  //
  ++index[0];
  outputPtr->TransformIndexToPhysicalPoint( index, tmpOutputPoint );
  tmpInputPoint = transformPtr->TransformPoint( tmpOutputPoint );
  inputPtr->TransformPhysicalPointToContinuousIndex( tmpInputPoint, tmpInputIndex );
  delta = tmpInputIndex - inputIndex;

  while ( !outIt.IsAtEnd() )
  {
    // Determine the continuous index of the first pixel of output
    // scanline when mapped to the input coordinate frame.
    //

    // First get the position of the pixel in the output coordinate frame
    index = outIt.GetIndex();
    outputPtr->TransformIndexToPhysicalPoint( index, outputPoint );

    // Compute corresponding input pixel continuous index, this index
    // will incremented in the scanline loop
    inputPoint = transformPtr->TransformPoint( outputPoint );
    inputPtr->TransformPhysicalPointToContinuousIndex( inputPoint, inputIndex );

    while ( !outIt.IsAtEndOfLine() )
    {
      PixelType  pixval;
      OutputType value;
      // Evaluate input at right position and copy to the output
      if ( m_Interpolator->IsInsideBuffer( inputIndex ) )
      {
        value = m_Interpolator->EvaluateAtContinuousIndex( inputIndex );
        pixval = this->CastPixelWithBoundsChecking( value, minOutputValue, maxOutputValue );
        outIt.Set( pixval );
      }
      else
      {
        if ( m_Extrapolator.IsNull() )
        {
          outIt.Set( defaultValue ); // default background value
        }
        else
        {
          value = m_Extrapolator->EvaluateAtContinuousIndex( inputIndex );
          pixval = this->CastPixelWithBoundsChecking( value, minOutputValue, maxOutputValue );
          outIt.Set( pixval );
        }
      }

      ++outIt;
      inputIndex += delta;
    }
    progress.CompletedPixel();
    outIt.NextLine();
  } // while( !outIt.IsAtEnd() )
}

/**
 * Inform pipeline of necessary input image region
 *
 * Determining the actual input region is non-trivial, especially
 * when we cannot assume anything about the transform being used.
 * So we do the easy thing and request the entire input image.
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::GenerateInputRequestedRegion()
{
  // call the superclass's implementation of this method
  Superclass::GenerateInputRequestedRegion();

  if ( !this->GetInput() )
  {
    return;
  }

  // get pointers to the input and output
  InputImagePointer inputPtr = const_cast< TInputImage * >( this->GetInput() );

  // Request the entire input image
  inputPtr->SetRequestedRegionToLargestPossibleRegion();
}

/**
 * Inform pipeline of required output region
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
void
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType,
                     TTransformPrecisionType >::GenerateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();

  // get pointers to the input and output
  OutputImageType * outputPtr = this->GetOutput();
  if ( !outputPtr )
  {
    return;
  }

  const ReferenceImageBaseType * referenceImage = this->GetReferenceImage();

  // Set the size of the output region
  if ( m_UseReferenceImage && referenceImage )
  {
    outputPtr->SetLargestPossibleRegion( referenceImage->GetLargestPossibleRegion() );
  }
  else
  {
    typename TOutputImage::RegionType outputLargestPossibleRegion;
    outputLargestPossibleRegion.SetSize( m_Size );
    outputLargestPossibleRegion.SetIndex( m_OutputStartIndex );
    outputPtr->SetLargestPossibleRegion( outputLargestPossibleRegion );
  }

  // Set spacing and origin
  if ( m_UseReferenceImage && referenceImage )
  {
    outputPtr->SetSpacing( referenceImage->GetSpacing() );
    outputPtr->SetOrigin( referenceImage->GetOrigin() );
    outputPtr->SetDirection( referenceImage->GetDirection() );
  }
  else
  {
    outputPtr->SetSpacing( m_OutputSpacing );
    outputPtr->SetOrigin( m_OutputOrigin );
    outputPtr->SetDirection( m_OutputDirection );
  }
}

/**
 * Verify if any of the components has been modified.
 */
template < typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType,
           typename TTransformPrecisionType >
ModifiedTimeType
ResampleImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::GetMTime(
  void ) const
{
  ModifiedTimeType latestTime = Object::GetMTime();

  if ( m_Interpolator )
  {
    if ( latestTime < m_Interpolator->GetMTime() )
    {
      latestTime = m_Interpolator->GetMTime();
    }
  }

  return latestTime;
}

} // end namespace itk

#endif
