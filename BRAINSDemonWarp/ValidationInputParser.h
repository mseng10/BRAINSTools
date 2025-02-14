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
#ifndef __ValidationInputParser_h
#define __ValidationInputParser_h

#include <string>
#include "itkObjectFactory.h"
#include "itkObject.h"
#include "itkFixedArray.h"
#include "itkArray.h"

#include "itkVersorRigid3DTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include <itkBSplineTransform.h>

namespace itk
{
// 6 Parameters
using VersorRigid3DTransformType = itk::VersorRigid3DTransform< double >;
// 9 Parameters
using ScaleVersor3DTransformType = itk::ScaleVersor3DTransform< double >;
// 15 Parameters
using ScaleSkewVersor3DTransformType = itk::ScaleSkewVersor3DTransform< double >;
// 12 Parameters
using AffineTransformType = itk::AffineTransform< double, 3 >;
/** \class ValidationInputParser
 *
 * This component parse an input parameter file for a simple
 * atlas based segmentation application.
 *
 * This class is activated by method Execute().
 *
 * Inputs:
 *  - altas image name
 *  - subject image name
 *  - the  parameter filename
 *
 *
 * Outputs:
 *  - pointer to the subject (fixed) image
 *  - pointer to the atlas (moving) image
 *  - the number of histogram levels to use
 *  - the number of histogram match points to use
 *  - the number of levels
 *  - the number of iterations at each level
 *  - the fixed image starting shrink factors
 *  - the moving image starting shrink factors
 *
 */

template < typename TImage >
class ValidationInputParser : public Object
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( ValidationInputParser );

  /** Standard class type alias. */
  using Self = ValidationInputParser;
  using Superclass = Object;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  /** Run-time type information (and related methods). */
  itkTypeMacro( ValidationInputParser, Object );

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Image Type. */
  using ImageType = TImage;
  using ImagePointer = typename ImageType::Pointer;

  /** Image dimension enumeration. */
  static constexpr unsigned int ImageDimension = TImage::ImageDimension;
  static constexpr unsigned int SplineOrder = 3;

  /** Transform Types. */
  using VersorRigid3DTransformType = VersorRigid3DTransform< double >;
  using ScaleSkewVersor3DTransformType = ScaleSkewVersor3DTransform< double >;
  using AffineTransformType = AffineTransform< double, Self::ImageDimension >;

  using CoordinateRepType = double;
  using BSplineTransformType =
    typename itk::BSplineTransform< CoordinateRepType, Self::ImageDimension, Self::SplineOrder >;

  /** Displacement field value type. */
  using FieldValueType = float;

  /** Displacement field pixel type. */
  using FieldPixelType = Vector< FieldValueType, Self::ImageDimension >;

  /** Displacement field type. */
  using TDisplacementField = Image< FieldPixelType, Self::ImageDimension >;

  /** ShrinkFactors type. */
  using ShrinkFactorsType = FixedArray< unsigned int, Self::ImageDimension >;

  /** IterationArray type. */
  using IterationsArrayType = Array< unsigned int >;

  /** Set the atlas patient. */
  itkSetStringMacro( TheMovingImageFilename );

  /** Set the subject patient. */
  itkSetStringMacro( TheFixedImageFilename );

  /** Set the initial Displacement Field one of 3 ways. */
  itkSetStringMacro( InitialDisplacementFieldFilename );
  itkSetStringMacro( InitialCoefficientFilename );
  itkSetStringMacro( InitialTransformFilename );

  /** Set input parameter file name. */
  //            itkSetStringMacro( ParameterFilename );

  /** Parse the input file. */
  void
  Execute();

  /** Get pointer to the atlas image. */
  itkGetModifiableObjectMacro( TheMovingImage, ImageType );
  itkGetModifiableObjectMacro( TheFixedImage, ImageType );

  /**force Centered Image.*/
  itkSetMacro( ForceCoronalZeroOrigin, bool );
  itkGetConstMacro( ForceCoronalZeroOrigin, bool );

  /** Get pointer to the subject image. */
  itkGetModifiableObjectMacro( InitialDisplacementField, TDisplacementField );

  /** Get the number of histogram bins. */
  itkGetConstMacro( NumberOfHistogramLevels, unsigned long );
  itkSetMacro( NumberOfHistogramLevels, unsigned long );

  /** Get the number of match points. */
  itkGetConstMacro( NumberOfMatchPoints, unsigned long );
  itkSetMacro( NumberOfMatchPoints, unsigned long );

  /** Get the number of levels. */
  itkGetMacro( NumberOfLevels, unsigned short );
  itkSetMacro( NumberOfLevels, unsigned short );

  /**Set Debug mode*/
  itkSetMacro( OutDebug, bool );
  itkGetConstMacro( OutDebug, bool );

  /** Get the atlas image starting shrink factors. */
  itkGetConstReferenceMacro( TheMovingImageShrinkFactors, ShrinkFactorsType );
  void
  SetTheMovingImageShrinkFactors( const ShrinkFactorsType & shrinkfactors )
  {
    this->m_TheMovingImageShrinkFactors = shrinkfactors;
  }

  /** Get the subject image starting shrink factors. */
  itkGetConstReferenceMacro( TheFixedImageShrinkFactors, ShrinkFactorsType );
  void
  SetTheFixedImageShrinkFactors( const ShrinkFactorsType & shrinkfactors )
  {
    this->m_TheFixedImageShrinkFactors = shrinkfactors;
  }

  /** Get the number of iterations at each level. */
  itkGetConstReferenceMacro( NumberOfIterations, IterationsArrayType );
  void
  SetNumberOfIterations( const IterationsArrayType & iterations )
  {
    m_NumberOfIterations = iterations;
  }

protected:
  ValidationInputParser();
  ~ValidationInputParser() override {}

private:
  std::string m_TheMovingImageFilename;
  std::string m_TheFixedImageFilename;
  std::string m_InitialDisplacementFieldFilename;
  std::string m_InitialCoefficientFilename;
  std::string m_InitialTransformFilename;
  std::string m_ParameterFilename;

  typename ImageType::Pointer m_TheMovingImage;
  typename ImageType::Pointer m_TheFixedImage;

  bool m_ForceCoronalZeroOrigin;
  //  bool                          m_HistogramMatching;
  bool m_OutDebug;

  typename TDisplacementField::Pointer m_InitialDisplacementField;

  unsigned long       m_NumberOfHistogramLevels;
  unsigned long       m_NumberOfMatchPoints;
  unsigned short      m_NumberOfLevels;
  ShrinkFactorsType   m_TheMovingImageShrinkFactors;
  ShrinkFactorsType   m_TheFixedImageShrinkFactors;
  IterationsArrayType m_NumberOfIterations;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "ValidationInputParser.hxx"
#endif

#endif
