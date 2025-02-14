//
// Created by Leinoff, Alexander on 5/20/16.
//

#include <itkTransformToDisplacementFieldFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <BRAINSRefacerCLP.h>
#include <Slicer3LandmarkIO.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkBSplineTransform.h>
#include <itkResampleImageFilter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkDivideImageFilter.h>
#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkTransformFileWriter.h>
#include <itkMultiplyImageFilter.h>
#include <itkBSplineTransformInitializer.h>
#include <itkComposeImageFilter.h>
#include <itkDisplacementFieldTransform.h>
#include <itkSubtractImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkTransformFileReader.h>
#include <itkCompositeTransform.h>
#include <itkTransformFactory.h>

#include "CreateRandomBSpline.h"
#include "CombineBSplineWithDisplacement.h"
#include "MaskFromLandmarksFilter.h"
#include "MaskFromLabelMapFilter.h"
#include "BRAINSRefacerUtilityFunctions.hxx"

#include "BRAINSRefacerUtilityFunctions.hxx" //Why does this not need to be included??


int
main( int argc, char ** argv )
{
  PARSE_ARGS;

  if ( debug_Refacer )
    verbose_Refacer = true; // debug should always be verbose

  // check for multiple preset switches
  int sumOptions = static_cast< int >( defaultface ) + static_cast< int >( birdface ) + static_cast< int >( babyface ) +
                   static_cast< int >( reusebspline ) + static_cast< int >( setMersenneSeed );
  if ( sumOptions > 1 )
  {
    std::cerr << "Too many presets specified. using default preset." << std::endl;
    reusebspline = false;
    birdface = false;
    babyface = false;
    defaultface = true;
  }

  if ( defaultface )
  {
    if ( verbose_Refacer )
    {
      std::cout << "Using \"DefaultFace\" parameters, explicitly given controls options will be ignored" << std::endl;
    }
    maxRandom = 5;
    minRandom = -5;
    bsplineControlPoints = 25;
    scaleDistanceMap = 0.05;
  }
  if ( birdface ) // default parameters
  {
    if ( verbose_Refacer )
    {
      std::cout << "Using \"BirdFace\" parameters, explicitly given controls options will be ignored" << std::endl;
    }
    maxRandom = 5;
    minRandom = -3;
    bsplineControlPoints = 24;
    scaleDistanceMap = 0.05;
  }
  if ( babyface ) // default parameters
  {
    if ( verbose_Refacer )
    {
      std::cout << "Using \"BabyFace\" parameters, explicitly given controls options will be ignored" << std::endl;
    }
    maxRandom = 0;
    minRandom = -8;
    bsplineControlPoints = 22;
    scaleDistanceMap = 0.05;
  }
  if ( setMersenneSeed ) // default parameters
  {
    if ( verbose_Refacer )
    {
      std::cout << "Using predetermined mersenne seed parameters, the default range and number of control points will ";
      std::cout << "be used, and any other explicitly given control options will be ignored" << std::endl;
    }
    // these are (and should be) the default parameters. If different parameters are used with the same seed, then
    // you'll
    // get a different result.
    maxRandom = 5;
    minRandom = -5;
    bsplineControlPoints = 25;
    scaleDistanceMap = 0.05;
  }

  // Basic typedef's
  using ProcessPixelType = double;
  constexpr unsigned int Dimension = 3;
  using ProcessImageType = itk::Image< ProcessPixelType, Dimension >;

  // Read in subject image
  using ImageReaderType = itk::ImageFileReader< ProcessImageType >;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName( inputImage );
  ProcessImageType::Pointer subject = imageReader->GetOutput();
  imageReader->Update();

  // Read the original image format

  itk::ImageIOBase::Pointer imageReaderIOBase = imageReader->GetModifiableImageIO();
  imageReaderIOBase->ReadImageInformation();
  // Note that in ImageIOBase pixel type refers to vector/scalar
  // component type refers to INT, LONG, FLOAT, etc.
  using IOComponentType = itk::ImageIOBase::IOComponentType;
  const IOComponentType originalComponentType_ENUM = imageReaderIOBase->GetComponentType();


  using ImageMaskType = itk::Image< unsigned char, Dimension >;
  ImageMaskType::Pointer brainMask = ImageMaskType::New();

  // Read in the atlas label file
  using LabelAtlasType = itk::Image< ProcessPixelType, Dimension >;
  using LabelAtlasReaderType = itk::ImageFileReader< LabelAtlasType >;
  LabelAtlasReaderType::Pointer labelAtlasReader = LabelAtlasReaderType::New();
  //  using MaskFilterType = itk::BinaryThresholdImageFilter<LabelAtlasType, ImageMaskType>;
  // MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  LabelAtlasType::Pointer labelAtlasReaderOutput = LabelAtlasType::New();


  if ( labelmapSwitch == false )
  {

    // Read in the landmarks file
    LandmarksMapType myLandmarks = ReadSlicer3toITKLmk( landmarks );

    using MaskFromLandmarksFilterType = MaskFromLandmarksFilter< ProcessImageType, ImageMaskType >;
    MaskFromLandmarksFilterType::Pointer masker = MaskFromLandmarksFilterType::New();
    std::cout << "Generating mask from landmarks ..." << std::endl;
    masker->SetInput( subject );
    masker->SetDebug( debug_Refacer );
    masker->SetVerbose( verbose_Refacer );
    masker->SetLandmarksFileName( landmarks );

    // Write to a file
    brainMask = masker->GetOutput();
    masker->Update();
  }
  else
  {
    std::cout << "Generating mask from labelmap" << std::endl;

    labelAtlasReader->SetFileName( labelmap );
    labelAtlasReaderOutput = labelAtlasReader->GetOutput();

    using MaskFromLabelMapFilterType = MaskFromLabelMapFilter< ProcessImageType, LabelAtlasType, ImageMaskType >;
    MaskFromLabelMapFilterType::Pointer maskFilter = MaskFromLabelMapFilterType::New();
    maskFilter->SetReferenceImage( subject );
    maskFilter->SetInputAtlas( labelAtlasReader->GetOutput() );

    brainMask = maskFilter->GetOutput();
    brainMask->Update();
  }
  if ( debug_Refacer )
  {
    WriteImage< ImageMaskType >( outputMask, brainMask );
  }
  // Get a distance map to the Brain region:
  using DistanceMapFilter = itk::SignedMaurerDistanceMapImageFilter< ImageMaskType, ProcessImageType >;

  DistanceMapFilter::Pointer distanceMapFilter = DistanceMapFilter::New();
  std::cout << "Calculating distance map ..." << std::endl;
  distanceMapFilter->SetInput( brainMask );
  distanceMapFilter->SetSquaredDistance( false );
  ProcessImageType::Pointer myDistanceMapFilterImage = distanceMapFilter->GetOutput();
  distanceMapFilter->Update();

  // make the distance map unsigned:
  using ThresholdFilterType = itk::ThresholdImageFilter< ProcessImageType >;
  ThresholdFilterType::Pointer distanceThreshold = ThresholdFilterType::New();
  distanceThreshold->SetInput( distanceMapFilter->GetOutput() );
  distanceThreshold->SetLower( 0.0 );
  distanceThreshold->SetUpper( 4096 ); // INFO: This should be changed to the max pixel value for the image type??? or
                                       // will we always be using double for calculations??
  distanceThreshold->SetOutsideValue( 0.0 );

  // Write the distance map to a file so we can see what it did:
  if ( debug_Refacer )
  {
    WriteImage( distanceMapFileName, distanceThreshold->GetOutput() );
  }
  ProcessImageType::Pointer myDistanceMapPreScaled = distanceThreshold->GetOutput();
  distanceThreshold->Update();

  // Try to scale distance map
  using ScalingFilterType = itk::MultiplyImageFilter< ProcessImageType, ProcessImageType, ProcessImageType >;
  ScalingFilterType::Pointer distanceMapScaler = ScalingFilterType::New();
  std::cout << "Scaling distance map ..." << std::endl;
  distanceMapScaler->SetInput( myDistanceMapPreScaled );
  distanceMapScaler->SetConstant( scaleDistanceMap );

  ProcessImageType::Pointer scaledDistanceMap = distanceMapScaler->GetOutput();
  distanceMapScaler->Update();

  // Perform some kind of BSpline on Image
  constexpr int BSplineOrder = 3;

  using Test = CreateRandomBSpline< ProcessImageType, ProcessPixelType, Dimension, BSplineOrder >
                          BSplineCreator; //, BSTransformType>;
  BSplineCreator::Pointer bSplineCreator = BSplineCreator::New();
  using BSTransformType = itk::BSplineTransform< ProcessPixelType, Dimension, BSplineOrder >;
  BSTransformType::Pointer bSpline = BSTransformType::New();

  // Stuff for reading in bspline transform
  using TransformReaderType = itk::TransformFileReaderTemplate< double >;
  TransformReaderType::Pointer transformReader = TransformReaderType::New();


  if ( !reusebspline )
  {
    std::cout << "Generating brand new random BSPline" << std::endl;
    bSplineCreator->SetDebug( debug_Refacer );
    bSplineCreator->SetVerbose( verbose_Refacer );
    bSplineCreator->SetInput( subject );
    bSplineCreator->SetBSplineControlPoints( bsplineControlPoints );
    bSplineCreator->SetRandMax( maxRandom );
    bSplineCreator->SetRandMin( minRandom );
    if ( setMersenneSeed )
    {
      bSplineCreator->SetMersenneSeed( mersenneSeed );
    }
    if ( printMersenneSeed )
    {
      bSplineCreator->SetPrintMersenneSeed( true );
    }
    bSplineCreator->Update();
    bSpline = bSplineCreator->GetBSplineOutput();
    if ( debug_Refacer || saveTransform )
    {
      WriteTransform( bSplineFileName, bSpline );
    }
  }
  else if ( reusebspline )
  {
    std::cout << "Reusing BSpline" << std::endl;

    transformReader->SetFileName( previousBSplineFileName );

    // try catch for ioreader
    try
    {
      transformReader->Update();
    }
    catch ( itk::ExceptionObject & exception )
    {
      std::cerr << "Error while reading the transform file" << std::endl;
      std::cerr << exception << std::endl;
      return EXIT_FAILURE;
    }
    const TransformReaderType::TransformListType * transforms = transformReader->GetTransformList();
    using ReadCompositeTransformType = itk::CompositeTransform< double, 3 >;
    TransformReaderType::TransformListType::const_iterator comp_it = transforms->begin();
    if ( strcmp( ( *comp_it )->GetNameOfClass(), "BSplineTransform" ) != 0 )
    {
      std::cerr << "Invalid transform given" << std::endl;
      std::cerr << "Transform type given was: " << std::endl;
      std::cerr << ( *comp_it )->GetNameOfClass() << std::endl;
      std::cerr << "You should only supply BSplineTransforms" << std::endl;
      return EXIT_FAILURE;
    }

    ReadCompositeTransformType::Pointer compositeRead =
      static_cast< ReadCompositeTransformType * >( ( *comp_it ).GetPointer() );

    // create a new bspline with the params from the composite that we read in.
    bSpline->SetFixedParameters( compositeRead->GetFixedParameters() );
    bSpline->SetParameters( compositeRead->GetParameters() );
  }


  using VectorPixelType = itk::Vector< ProcessPixelType, Dimension >;
  using DisplacementFieldProcessImageType = itk::Image< VectorPixelType, Dimension >;

  using CombinerType =
    CombineBSplineWithDisplacement< ProcessImageType, DisplacementFieldProcessImageType, ProcessPixelType, 3, 3 >;

  CombinerType::Pointer combiner = CombinerType::New();

  std::cout << "Combining bspline with displacement ..." << std::endl;

  combiner->SetDebug( debug_Refacer );
  combiner->SetVerbose( verbose_Refacer );
  combiner->SetBSplineInput( bSpline );
  combiner->SetInput( subject );
  combiner->SetDistanceMap( distanceMapScaler->GetOutput() );
  combiner->Update();

  // write the new displacement image
  DisplacementFieldProcessImageType * composedDisplacementField_rawPtr = combiner->GetComposedImage();
  if ( debug_Refacer )
  {
    WriteImage( smoothDisplacementName, composedDisplacementField_rawPtr );
  }
  using FinalTransformType = itk::DisplacementFieldTransform< ProcessPixelType, Dimension >;
  FinalTransformType::Pointer finalTransform = FinalTransformType::New();
  finalTransform->SetDisplacementField( composedDisplacementField_rawPtr );

  if ( debug_Refacer )
  {
    WriteTransform( finalTransformFileName, finalTransform );
  }

  // Apply transform to image with resampler:
  using ResampleFilterType = itk::ResampleImageFilter< ProcessImageType, ProcessImageType >;
  ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  using InterpolatorType = itk::LinearInterpolateImageFunction< ProcessImageType, ProcessPixelType >;
  InterpolatorType::Pointer interpolater = InterpolatorType::New();

  ProcessImageType::RegionType subjectRegion = subject->GetBufferedRegion();

  std::cout << "Refacing image ..." << std::endl;

  resampler->SetInterpolator( interpolater );
  resampler->SetReferenceImage( imageReader->GetOutput() );
  resampler->UseReferenceImageOn();
  resampler->SetOutputStartIndex( subjectRegion.GetIndex() );

  resampler->SetInput( imageReader->GetOutput() );
  resampler->SetTransform( finalTransform );

  // WriteImage(deformedImageName, resampler->GetOutput());
  ConvertAndSave< ProcessImageType, Dimension >(
    deformedImageName, resampler->GetOutput(), originalComponentType_ENUM );

  // write the difference Image
  if ( debug_Refacer )
  {
    // Get the difference image
    using SubtractFilter = itk::SubtractImageFilter< ProcessImageType, ProcessImageType >;
    SubtractFilter::Pointer subtractFilter = SubtractFilter::New();
    subtractFilter->SetInput1( subject );
    subtractFilter->SetInput2( resampler->GetOutput() );
    WriteImage( diffImageName, subtractFilter->GetOutput() );
  }

  std::cout << "done" << std::endl;

  return EXIT_SUCCESS;
}
