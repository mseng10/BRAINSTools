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
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkESMDemonsRegistrationFunction.h,v $
 *  Language:  C++
 *  Date:      $Date: 2008-07-04 22:54:43 $
 *  Version:   $Revision: 1.2 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkVectorESMDemonsRegistrationFunction_h
#define __itkVectorESMDemonsRegistrationFunction_h

#include "itkPDEDeformableRegistrationFunction.h"
#include "itkPoint.h"
#include "itkCovariantVector.h"
#include "itkInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkCentralDifferenceImageFunction.h"
#include "itkWarpImageFilter.h"
#include "itkVectorImageToImageAdaptor.h"

namespace itk
{
/**
 * \class ESMDemonsRegistrationFunction
 *
 * \brief Fast implementation of the symmetric demons registration force
 *
 * This class provides a substantially faster implementation of the
 * symmetric demons registration force. Speed is improved by keeping
 * a deformed copy of the moving image for gradient evaluation.
 *
 * Symmetric forces simply means using the mean of the gradient
 * of the fixed image and the gradient of the warped moving
 * image.
 *
 * Note that this class also enables the use of fixed, mapped moving
 * and warped moving images forces by using a call to SetUseGradientType
 *
 * The moving image should not be saturated. We indeed use
 * NumericTraits<MovingPixelType>::Max() as a special value.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 * This implementation was taken from the Insight Journal paper:
 * http://hdl.handle.net/1926/510
 *
 * \sa SymmetricForcesDemonsRegistrationFunction
 * \sa SymmetricForcesDemonsRegistrationFilter
 * \sa DemonsRegistrationFilter
 * \sa DemonsRegistrationFunction
 * \ingroup FiniteDifferenceFunctions
 *
 */
template < typename TFixedImage, typename TMovingImage, typename TDisplacementField >
class VectorESMDemonsRegistrationFunction
  : public PDEDeformableRegistrationFunction< TFixedImage, TMovingImage, TDisplacementField >
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( VectorESMDemonsRegistrationFunction );

  /** Standard class type alias. */
  using Self = VectorESMDemonsRegistrationFunction;
  using Superclass = PDEDeformableRegistrationFunction< TFixedImage, TMovingImage, TDisplacementField >;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( VectorESMDemonsRegistrationFunction, PDEDeformableRegistrationFunction );

  /** MovingImage image type. */
  using VectorMovingImageType = TMovingImage;
  using VectorMovingImagePointer = typename VectorMovingImageType::ConstPointer;

  /** FixedImage image type. */
  using VectorFixedImageType = TFixedImage;
  using VectorFixedImagePointer = typename VectorFixedImageType::ConstPointer;

  /** Inherit some enums from the superclass. */
  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  /** MovingImage image type. */
  //  using MovingImageType = typename Superclass::MovingImageType;
  using MovingImageType = itk::Image< float, Self::ImageDimension >;
  //  using MovingImagePointer = typename Superclass::MovingImagePointer;
  using MovingImagePointer = typename MovingImageType::Pointer;
  using MovingPixelType = typename MovingImageType::PixelType;

  /** FixedImage image type. */
  //  using FixedImageType = typename Superclass::FixedImageType;
  using FixedImageType = itk::Image< float, Self::ImageDimension >;
  using FixedImagePointer = typename FixedImageType::Pointer;
  //  using FixedImagePointer = typename Superclass::FixedImagePointer;
  using IndexType = typename FixedImageType::IndexType;
  using SizeType = typename FixedImageType::SizeType;
  using SpacingType = typename FixedImageType::SpacingType;
  using DirectionType = typename FixedImageType::DirectionType;

  /** Displacement field type. */
  using DisplacementFieldType = typename Superclass::DisplacementFieldType;
  using DisplacementFieldPointer = typename DisplacementFieldType::Pointer;

  /** Inherit some enums from the superclass. */
  using PixelType = typename Superclass::PixelType;
  using RadiusType = typename Superclass::RadiusType;
  using NeighborhoodType = typename Superclass::NeighborhoodType;
  using FloatOffsetType = typename Superclass::FloatOffsetType;
  using TimeStepType = typename Superclass::TimeStepType;

  using AdaptorType = itk::VectorImageToImageAdaptor< MovingPixelType, Self::ImageDimension >;

  /** Interpolator type. */
  using CoordRepType = double;
  using InterpolatorType = InterpolateImageFunction< AdaptorType, CoordRepType >;
  using InterpolatorPointer = typename InterpolatorType::Pointer;
  using PointType = typename InterpolatorType::PointType;
  using DefaultInterpolatorType = LinearInterpolateImageFunction< AdaptorType, CoordRepType >;

  /** Warper type */
  using WarperType = WarpImageFilter< AdaptorType, MovingImageType, DisplacementFieldType >;
  using WarperPointer = typename WarperType::Pointer;

  /** Covariant vector type. */
  using CovariantVectorType = CovariantVector< double, Self::ImageDimension >;

  /** Fixed image gradient calculator type. */
  using GradientCalculatorType = CentralDifferenceImageFunction< AdaptorType >;
  using GradientCalculatorPointer = typename GradientCalculatorType::Pointer;

  /** Moving image gradient (unwarped) calculator type. */
  using MovingImageGradientCalculatorType = CentralDifferenceImageFunction< AdaptorType, CoordRepType >;
  typedef typename MovingImageGradientCalculatorType::Pointer MovingImageGradientCalculatorPointer;

  /** Set the moving image interpolator. */
  void
  SetMovingImageInterpolator( InterpolatorType * ptr )
  {
    m_MovingImageInterpolator = ptr;
    m_MovingImageWarper->SetInterpolator( ptr );
  }

  /** Get the moving image interpolator. */
  InterpolatorType *
  GetMovingImageInterpolator( void )
  {
    return m_MovingImageInterpolator;
  }

  /** This class uses a constant timestep of 1. */
  TimeStepType
  ComputeGlobalTimeStep( void * itkNotUsed( GlobalData ) ) const override
  {
    return m_TimeStep;
  }

  /** Return a pointer to a global data structure that is passed to
   * this object from the solver at each calculation.  */
  void *
  GetGlobalDataPointer() const override
  {
    GlobalDataStruct * global = new GlobalDataStruct();

    global->m_SumOfSquaredDifference = 0.0;
    global->m_NumberOfPixelsProcessed = 0L;
    global->m_SumOfSquaredChange = 0;
    return global;
  }

  /** Release memory for global data structure. */
  void
  ReleaseGlobalDataPointer( void * GlobalData ) const override;

  /** Set the object's state before each iteration. */
  void
  InitializeIteration() override;

  /** This method is called by a finite difference solver image filter at
   * each pixel that does not lie on a data set boundary */
  PixelType
  ComputeUpdate( const NeighborhoodType & neighborhood, void * globalData,
                 const FloatOffsetType & offset = FloatOffsetType( 0.0 ) ) override;

  /** Get the metric value. The metric value is the mean square difference
   * in intensity between the fixed image and transforming moving image
   * computed over the the overlapping region between the two images. */
  virtual double
  GetMetric() const
  {
    return m_Metric;
  }

  /** Get the rms change in deformation field. */
  virtual const double &
  GetRMSChange() const
  {
    return m_RMSChange;
  }

  /** Set/Get the threshold below which the absolute difference of
   * intensity yields a match. When the intensities match between a
   * moving and fixed image pixel, the update vector (for that
   * iteration) will be the zero vector. Default is 0.001. */
  virtual void
  SetIntensityDifferenceThreshold( double );

  virtual double
  GetIntensityDifferenceThreshold() const;

  /** Set/Get the maximum update step length. In Thirion this is 0.5.
   *  Setting it to 0 implies no restriction (beware of numerical
   *  instability in this case. */
  virtual void
  SetMaximumUpdateStepLength( double sm )
  {
    this->m_MaximumUpdateStepLength = sm;
  }

  virtual double
  GetMaximumUpdateStepLength() const
  {
    return this->m_MaximumUpdateStepLength;
  }

  /** Type of available image forces */
  enum GradientType
  {
    Symmetric = 0,
    Fixed,
    WarpedMoving,
    MappedMoving
  };

  /** Set/Get the type of used image forces */
  virtual void
  SetUseGradientType( GradientType gtype )
  {
    m_UseGradientType = gtype;
  }

  virtual GradientType
  GetUseGradientType() const
  {
    return m_UseGradientType;
  }

  /** Set the moving image.  */
  void
  SetMovingImage( const VectorMovingImageType * ptr )
  {
    m_MovingImage = ptr;
  }

  /** Get the moving image. */
  const VectorMovingImageType *
  GetMovingImage( void ) const
  {
    return m_MovingImage;
  }

  /** Set the fixed image. */
  void
  SetFixedImage( const VectorFixedImageType * ptr )
  {
    m_FixedImage = ptr;
  }

  /** Get the fixed image. */
  const VectorFixedImageType *
  GetFixedImage( void ) const
  {
    return m_FixedImage;
  }

protected:
  VectorESMDemonsRegistrationFunction();
  ~VectorESMDemonsRegistrationFunction() override {}

  void
  PrintSelf( std::ostream & os, Indent indent ) const override;

  /** FixedImage image neighborhood iterator type. */
  using FixedImageNeighborhoodIteratorType = ConstNeighborhoodIterator< FixedImageType >;

  /** A global data type for this class of equation. Used to store
   * iterators for the fixed image. */
  struct GlobalDataStruct
  {
    double        m_SumOfSquaredDifference;
    unsigned long m_NumberOfPixelsProcessed;
    double        m_SumOfSquaredChange;
  };

private:
  VectorFixedImagePointer  m_FixedImage;
  VectorMovingImagePointer m_MovingImage;

  /** Cache fixed image information. */
  SpacingType   m_FixedImageSpacing;
  PointType     m_FixedImageOrigin;
  DirectionType m_FixedImageDirection;
  double        m_Normalizer;

  /** Function to compute derivatives of the fixed image. */
  GradientCalculatorPointer m_FixedImageGradientCalculator;

  /** Function to compute derivatives of the moving image (unwarped). */
  MovingImageGradientCalculatorPointer m_MappedMovingImageGradientCalculator;

  GradientType m_UseGradientType;

  /** Function to interpolate the moving image. */
  InterpolatorPointer m_MovingImageInterpolator;

  /** Filter to warp moving image for fast gradient computation. */
  WarperPointer m_MovingImageWarper;

  /** The global timestep. */
  TimeStepType m_TimeStep;

  /** Threshold below which the denominator term is considered zero. */
  double m_DenominatorThreshold;

  /** Threshold below which two intensity value are assumed to match. */
  double m_IntensityDifferenceThreshold;

  /** Maximum update step length in pixels (default is 0.5 as in Thirion). */
  double m_MaximumUpdateStepLength;

  /** The metric value is the mean square difference in intensity between
   * the fixed image and transforming moving image computed over the
   * the overlapping region between the two images. */
  mutable double        m_Metric;
  mutable double        m_SumOfSquaredDifference;
  mutable unsigned long m_NumberOfPixelsProcessed;
  mutable double        m_RMSChange;
  mutable double        m_SumOfSquaredChange;

  /** Mutex lock to protect modification to metric. */
  mutable std::mutex m_MetricCalculationLock;

  std::vector< WarperPointer >                        m_MovingImageWarperVector;
  std::vector< InterpolatorPointer >                  m_MovingImageInterpolatorVector;
  std::vector< GradientCalculatorPointer >            m_FixedImageGradientCalculatorVector;
  std::vector< MovingImageGradientCalculatorPointer > m_MappedMovingImageGradientCalculatorVector;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkVectorESMDemonsRegistrationFunction.hxx"
#endif

#endif
