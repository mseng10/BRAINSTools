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
/*=========================================================================
 Author: Wei Lu, Ali Ghayoor, Hans Johnson
 SINAPSE
 University of Iowa, 2010, 2012, 2013
 =========================================================================*/

#include "PrepareOutputImages.h"
#include "landmarksConstellationDetector.h"
#include "BRAINSConstellationDetector2.h"
#include "itkOrthogonalize3DRotationMatrix.h"
#include "itkLandmarkBasedTransformInitializer.h"

#include "itkResampleInPlaceImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkCastImageFilter.h"
#include "GenericTransformImage.h"
#include "landmarksConstellationDetector.h"

namespace itk
{


template < typename TInputImage, typename TOutputImage >
BRAINSConstellationDetector2< TInputImage, TOutputImage >::BRAINSConstellationDetector2()
{
  /** Essential Parameters */
  // Inputs
  this->m_Transform = "";
  this->m_InputTemplateModel = "";
  this->m_MspQualityLevel = 2;
  this->m_OtsuPercentileThreshold = 0.01;
  this->m_AcLowerBound = 1000.0;
  this->m_CutOutHeadInOutputVolume = false;
  this->m_RescaleIntensities = false;
  this->m_TrimRescaledIntensities = 4.4172;
  this->m_RescaleIntensitiesOutputRange.push_back( 40 );
  this->m_RescaleIntensitiesOutputRange.push_back( 4000 );
  this->m_BackgroundFillValueString = "0";
  this->m_InterpolationMode = "Linear";

  this->m_OriginalInputImage = nullptr;
  this->m_orig2eyeFixed_img_tfm = nullptr;

  m_HoughEyeFailure = false;

  this->m_LlsMatrices.clear();
  this->m_LlsMeans.clear();

  this->m_ACMean.Fill( -123 );
  this->m_SearchRadii.clear();

  // Outputs
  this->m_OrigToACPCVersorTransform = nullptr;
  this->m_ACPCToOrigVersorTransform = nullptr;
  this->m_AlignedPoints.clear();

  this->m_OutputImage = nullptr;
  this->m_OutputResampledImage = nullptr;
  this->m_OutputUntransformedClippedVolume = nullptr;
  this->m_CleanedIntensityOriginalInputImage = nullptr;

  /** Advanced parameters */
  /** Manual Override */
  // Inputs
  this->m_force_orig_lmk_ACPointLPS.clear();
  this->m_force_orig_lmk_PCPointLPS.clear();
  this->m_force_orig_lmk_VN4PointLPS.clear();
  this->m_force_orig_lmk_RPPointLPS.clear();

  /** Model Override */
  // Inputs
  this->m_RadiusMPJ = -1.;
  this->m_RadiusAC = -1.;
  this->m_RadiusPC = -1.;
  this->m_RadiusVN4 = -1.;

  /** Debug Options */
  // Inputs
  this->m_Debug = false;
  this->m_Verbose = false;
  this->m_WritedebuggingImagesLevel = 0;
  this->m_WriteBranded2DImage = "";
  this->m_ResultsDir = "./";

  this->m_atlasVolume = "";
  this->m_atlasLandmarks = "";
  this->m_atlasLandmarkWeights = "";
}

template < typename TInputImage, typename TOutputImage >
void
BRAINSConstellationDetector2< TInputImage, TOutputImage >::GenerateData()
{
  // file pointer for opening the setup file
  // /////////////////////////////////////////////////////////////////////////////////////////////
  LMC::globalverboseFlag = this->m_Verbose;
  globalImagedebugLevel = this->m_WritedebuggingImagesLevel;

  // /////////////////////////////////////////////////////////////////////////////////////////////
  short BackgroundFillValue;
  if ( this->m_BackgroundFillValueString == std::string( "BIGNEG" ) )
  {
    BackgroundFillValue = -32768;
  }
  else
  {
    BackgroundFillValue = std::stoi( this->m_BackgroundFillValueString.c_str() );
  }
  // /////////////////////////////////////////////////////////////////////////////////////////////
  // read information from the setup file, and initialize some variables
  landmarksConstellationModelIO myModel;
  myModel.ReadModelFile( this->m_InputTemplateModel );


  if ( LMC::globalverboseFlag )
  {
    std::cout << "Using Model File: " << this->m_InputTemplateModel << std::endl;
    myModel.PrintHeaderInfo();
  }

  // Override some landmarks by user
  vnl_vector< double > templateRadius; // in units of mm
  templateRadius.set_size( 4 );
  templateRadius[0] = ( this->m_RadiusMPJ <= 0 ) ? myModel.GetRadius( "RP" ) : this->m_RadiusMPJ;

  templateRadius[1] = ( this->m_RadiusAC <= 0 ) ? myModel.GetRadius( "AC" ) : this->m_RadiusAC;
  templateRadius[2] = ( this->m_RadiusPC <= 0 ) ? myModel.GetRadius( "PC" ) : this->m_RadiusPC;
  templateRadius[3] = ( this->m_RadiusVN4 <= 0 ) ? myModel.GetRadius( "VN4" ) : this->m_RadiusVN4;
  myModel.SetRadius( "RP", templateRadius[0] );
  myModel.SetRadius( "AC", templateRadius[1] );
  myModel.SetRadius( "PC", templateRadius[2] );
  myModel.SetRadius( "VN4", templateRadius[3] );

  SImageType::Pointer copyOfOriginalInputImage;
  {
    LandmarkIO::DuplicatorType::Pointer duplicator = LandmarkIO::DuplicatorType::New();
    duplicator->SetInputImage( this->m_OriginalInputImage );
    duplicator->Update();
    copyOfOriginalInputImage = duplicator->GetOutput();
  }

  // RPPC is a vector on the MSP that points from the RP point to the PC.
  //   RP------->PC
  // //////////////////////////////////////////////////////////////////////////

  if ( this->m_RescaleIntensities == true )
  {
    itk::StatisticsImageFilter< SImageType >::Pointer stats = itk::StatisticsImageFilter< SImageType >::New();
    stats->SetInput( copyOfOriginalInputImage );
    stats->Update();
    SImageType::PixelType minPixel( stats->GetMinimum() );
    SImageType::PixelType maxPixel( stats->GetMaximum() );

    if ( this->m_TrimRescaledIntensities > 0.0 )
    {
      // FUTUREINFO:  Ali Consider updating this
      //  REFACTOR: a histogram would be traditional here, but seem over-the-top;
      // I did this because it seemed to me if I knew mean, sigma, max and min,
      // then I know Something about extreme outliers.
      // Look at the setLowHigh function in landmarksConstellationCommon.h as a
      // possible replacement

      const double meanOrig( stats->GetMean() );
      const double sigmaOrig( stats->GetSigma() );

      // FUTUREINFO:  Ali Consider updating this
      // REFACTOR:  In percentiles, 0.0005 two-tailed has worked in the past.
      // It only makes sense to trim the upper bound since the lower bound would
      // most likely
      // represent a large region of air around the head.  But this is not so
      // when using a mask.
      // For one-tailed, an error of 0.001 corresponds to 3.29052 standard
      // deviations of normal.
      // For one-tailed, an error of 0.0001 corresponds to 3.8906 standard
      // deviations of normal.
      // For one-tailed, an error of 0.00001 corresponds to 4.4172 standard
      // deviations of normal.
      // Naturally, the constant should default at the command line, ...

      const double variationBound( ( maxPixel - meanOrig ) / sigmaOrig );
      const double trimBound( variationBound - this->m_TrimRescaledIntensities );
      if ( trimBound > 0.0 )
      {
        maxPixel = static_cast< SImageType::PixelType >( maxPixel - trimBound * sigmaOrig );
      }
    }

    itk::IntensityWindowingImageFilter< SImageType, SImageType >::Pointer remapIntensityFilter =
      itk::IntensityWindowingImageFilter< SImageType, SImageType >::New();
    remapIntensityFilter->SetInput( copyOfOriginalInputImage );
    remapIntensityFilter->SetOutputMaximum( this->m_RescaleIntensitiesOutputRange[1] );
    remapIntensityFilter->SetOutputMinimum( this->m_RescaleIntensitiesOutputRange[0] );
    remapIntensityFilter->SetWindowMinimum( minPixel );
    remapIntensityFilter->SetWindowMaximum( maxPixel );
    remapIntensityFilter->Update();

    this->m_CleanedIntensityOriginalInputImage = remapIntensityFilter->GetOutput();
  }
  else
  {
    this->m_CleanedIntensityOriginalInputImage = copyOfOriginalInputImage;
  }


  landmarksConstellationDetector myDetector( m_forced_orig_lmks );
  {
    // a little abuse of the eyeFixed_img_duplicator here
    LandmarkIO::DuplicatorType::Pointer eyeFixed_img_duplicator = LandmarkIO::DuplicatorType::New();
    // Use HoughEyeAlignedImage + HoughTransform as starting point.
    eyeFixed_img_duplicator->SetInputImage( this->GeteyeFixed_img().GetPointer() );
    eyeFixed_img_duplicator->Update();
    // The detector will use the output image after the Hough eye detector
    myDetector.SeteyeFixed_img( eyeFixed_img_duplicator->GetOutput() );
  }


  myDetector.SetInputTemplateModel( myModel );
  myDetector.SetLlsMatrices( this->m_LlsMatrices );
  myDetector.SetLlsMeans( this->m_LlsMeans );
  myDetector.SetSearchRadii( this->m_SearchRadii );
  myDetector.SetResultsDir( this->m_ResultsDir );
  myDetector.SetTemplateRadius( myModel.GetRadii() );
  myDetector.SetMSPQualityLevel( this->m_MspQualityLevel );
  myDetector.SetHoughEyeFailure( this->m_HoughEyeFailure );


  myDetector.Setorig2eyeFixed_img_tfm( this->m_orig2eyeFixed_img_tfm );

  myDetector.SetatlasVolume( this->GetatlasVolume() );
  myDetector.SetatlasLandmarks( this->GetatlasLandmarks() );
  myDetector.SetatlasLandmarkWeights( this->GetatlasLandmarkWeights() );

  myDetector.Compute( this->m_CleanedIntensityOriginalInputImage );
  this->m_OrigToACPCVersorTransform = myDetector.GetImageOrigToACPCVersorTransform();

  if ( LMC::globalverboseFlag )
  {
    std::cout << "VersorRotation: " << this->m_OrigToACPCVersorTransform->GetMatrix() << std::endl;
    std::cout << "itkVersorRigid3DTransform Parameters: " << this->m_OrigToACPCVersorTransform->GetParameters()
              << std::endl;
    std::cout << "itkVersorRigid3DTransform FixedParameters: "
              << this->m_OrigToACPCVersorTransform->GetFixedParameters() << std::endl;
    std::cout << "itkVersorRigid3DTransform GetCenter(): " << this->m_OrigToACPCVersorTransform->GetCenter()
              << std::endl;
    std::cout << "itkVersorRigid3DTransform GetTranslation(): " << this->m_OrigToACPCVersorTransform->GetTranslation()
              << std::endl;
    std::cout << "itkVersorRigid3DTransform GetMatrix(): " << this->m_OrigToACPCVersorTransform->GetMatrix()
              << std::endl;

    std::cout << "itkRigid3DTransform Parameters: " << this->m_OrigToACPCVersorTransform->GetParameters() << std::endl;
    std::cout << "itkRigid3DTransform FixedParameters: " << this->m_OrigToACPCVersorTransform->GetFixedParameters()
              << std::endl;
    std::cout << "itkRigid3DTransform GetCenter(): " << this->m_OrigToACPCVersorTransform->GetCenter() << std::endl;
    std::cout << "itkRigid3DTransform GetTranslation(): " << this->m_OrigToACPCVersorTransform->GetTranslation()
              << std::endl;
    std::cout << "itkRigid3DTransform GetMatrix(): " << this->m_OrigToACPCVersorTransform->GetMatrix() << std::endl;
    std::cout << "itkVersorRigid3DTransform: \n" << this->m_OrigToACPCVersorTransform << std::endl;
    std::cout << "itkRigid3DTransform: \n" << this->m_OrigToACPCVersorTransform << std::endl;
  }

  itk::PrepareOutputImages( this->m_OutputResampledImage,
                            this->m_OutputImage,
                            this->m_OutputUntransformedClippedVolume,
                            this->m_CleanedIntensityOriginalInputImage.GetPointer(), // Input RO
                            this->m_OrigToACPCVersorTransform.GetPointer(),          // Input RO
                            this->m_AcLowerBound,                                    // Input RO
                            BackgroundFillValue,                                     // Input RO
                            this->m_InterpolationMode,                               // Input RO
                            this->m_CutOutHeadInOutputVolume,                        // Input RO
                            this->m_OtsuPercentileThreshold                          // Input RO
  );


  if ( globalImagedebugLevel > 3 )
  {
    const SImageType::Pointer TaggedOriginalImage =
      myDetector.GetTaggedImage( this->m_CleanedIntensityOriginalInputImage );
    itkUtil::WriteImage< SImageType >( TaggedOriginalImage, this->m_ResultsDir + "/TAGGED_POINTS.nii.gz" );
    {
      SImageType::Pointer isoTaggedImage =
        TransformResample< SImageType, SImageType >( TaggedOriginalImage.GetPointer(),
                                                     MakeIsoTropicReferenceImage().GetPointer(),
                                                     BackgroundFillValue,
                                                     GetInterpolatorFromString< SImageType >( "Linear" ).GetPointer(),
                                                     this->m_OrigToACPCVersorTransform.GetPointer() );
      itkUtil::WriteImage< SImageType >( isoTaggedImage, this->m_ResultsDir + "/ISO_Lmk_MSP.nii.gz" );
    }
    {
      SImageType::Pointer VersorisoTaggedImage =
        TransformResample< SImageType, SImageType >( TaggedOriginalImage.GetPointer(),
                                                     MakeIsoTropicReferenceImage().GetPointer(),
                                                     BackgroundFillValue,
                                                     GetInterpolatorFromString< SImageType >( "Linear" ).GetPointer(),
                                                     this->m_OrigToACPCVersorTransform.GetPointer() );
      itkUtil::WriteImage< SImageType >( VersorisoTaggedImage, this->m_ResultsDir + "/Versor_ISO_Lmk_MSP.nii.gz" );
    }
    {
      RigidTransformType::Pointer orig2msp_img_tfm = myDetector.Getorig2msp_img_tfm();
      SImageType::Pointer         msp_img =
        TransformResample< SImageType, SImageType >( TaggedOriginalImage.GetPointer(),
                                                     MakeIsoTropicReferenceImage().GetPointer(),
                                                     BackgroundFillValue,
                                                     GetInterpolatorFromString< SImageType >( "Linear" ).GetPointer(),
                                                     orig2msp_img_tfm.GetPointer() );
      itkUtil::WriteImage< SImageType >( msp_img, this->m_ResultsDir + "/RigidMSPImage_Lmk_MSP.nii.gz" );
    }
  }

  itk::ApplyInverseOfTransformToLandmarks( this->m_OrigToACPCVersorTransform.GetPointer(), // Input RO
                                           myDetector.Getorig_lmks_updated(),
                                           this->m_AlignedPoints );

  // Following is a mechanism to force BCD report failure if
  // transformed LE and RE are not in expected ranges with respect to AC point (0,0,0).
  std::vector< double > eyes_LR_range( 2 );
  eyes_LR_range[0] = 15.0;
  eyes_LR_range[1] = 45.0;

  if ( this->m_AlignedPoints["LE"][0] < eyes_LR_range[0] || this->m_AlignedPoints["LE"][0] > eyes_LR_range[1] ||
       this->m_AlignedPoints["RE"][0] > -eyes_LR_range[0] || this->m_AlignedPoints["RE"][0] < -eyes_LR_range[1] ||
       this->m_AlignedPoints["LE"][2] > 0 || this->m_AlignedPoints["RE"][2] > 0 )
  {
    const std::string EMSP_Fiducial_file_name( "EMSP.fcsv" );
    std::stringstream failureMessageStream( "" );
    if ( this->m_AlignedPoints["LE"][2] > 0 || this->m_AlignedPoints["RE"][2] > 0 )
    {
      failureMessageStream << "Eyes are normally lower than AC point in Superior-Inferior direction "
                           << "in MSP aligned space." << std::endl;
    }
    else
    {
      failureMessageStream << "Eyes are out of range in MSP aligned space." << std::endl
                           << "Normally in left-right direction, " << eyes_LR_range[0] << "<LE< " << eyes_LR_range[1]
                           << " and " << -eyes_LR_range[1] << "<RE<" << -eyes_LR_range[0] << std::endl
                           << "LE[0] = " << this->m_AlignedPoints["LE"][0]
                           << ", RE[0] = " << this->m_AlignedPoints["RE"][0] << std::endl;
    }
    WriteManualFixFiles( EMSP_Fiducial_file_name,
                         this->m_OutputResampledImage.GetPointer(),
                         this->m_ResultsDir,
                         this->m_AlignedPoints,
                         failureMessageStream.str(),
                         true );
  }

  if ( this->m_WriteBranded2DImage.compare( "" ) != 0 )
  {
    MakeBranded2DImage( this->m_OutputResampledImage.GetPointer(),
                        myDetector,
                        this->m_AlignedPoints["RP"],
                        this->m_AlignedPoints["AC"],
                        this->m_AlignedPoints["PC"],
                        this->m_AlignedPoints["VN4"],
                        this->m_AlignedPoints["CM"],
                        this->m_WriteBranded2DImage );
  }
  this->GraftOutput( this->m_OutputImage );
}

template < typename TInputImage, typename TOutputImage >
void
BRAINSConstellationDetector2< TInputImage, TOutputImage >::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
}

} // namespace itk
