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
#ifndef __itkQuadEdgeMeshScalarPixelValuesSmoothingFilter_h
#define __itkQuadEdgeMeshScalarPixelValuesSmoothingFilter_h

#include "itkQuadEdgeMeshToQuadEdgeMeshFilter.h"
#include "itkQuadEdgeMeshParamMatrixCoefficients.h"

namespace itk
{
/**
 * \class QuadEdgeMeshScalarPixelValuesSmoothingFilter
 * \brief This filter smooths scalar pixel values associated with points.
 *
 * This filter was based on the filter provided by
 * Arnaud Gelas, Alex Gouaillard and Sean Megason in their Insight Journal paper
 * http://hdl.handle.net/1926/1518
 * http://www.insight-journal.org/browse/publication/313
 *
 * The difference between this current filter and the one above is that this
 * filter smooths the values associated with the points (PointData) without
 * changing the actual positions of the points in space, while the filter above
 * smooths the point positions while leaving unchanged the pixel values
 * associated with the points.
 *
 * This filter expects the PixelType to be of Scalar type. At every node, the
 * scalar values be averaged using a weighted sum. The smoothing process is
 * performed for a user-specified number of iterations.
 *
 * A full description of this filter is available in the TMI paper:
 *
 * "Spherical Demons: Fast Diffeomorphic Landmark-Free Surface Registration"
 *
 * by
 *       B.T. Thomas Yeo, Mert R. Sabuncu, Tom Vercauteren,
 *       Nicholas Ayache, Bruce Fischl, Polina Golland.
 *
 * \sa QuadEdgeMeshVectorPixelValuesSmoothingFilter
 * \ingroup MeshFilters
 *
 */
template < typename TInputMesh, typename TOutputMesh >
class QuadEdgeMeshScalarPixelValuesSmoothingFilter : public QuadEdgeMeshToQuadEdgeMeshFilter< TInputMesh, TOutputMesh >
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( QuadEdgeMeshScalarPixelValuesSmoothingFilter );

  using Self = QuadEdgeMeshScalarPixelValuesSmoothingFilter;
  using Superclass = QuadEdgeMeshToQuadEdgeMeshFilter< TInputMesh, TOutputMesh >;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( QuadEdgeMeshScalarPixelValuesSmoothingFilter, QuadEdgeMeshToQuadEdgeMeshFilter );

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro( Self );

  using InputMeshType = TInputMesh;
  using InputMeshPointer = typename InputMeshType::Pointer;
  using InputPixelType = typename InputMeshType::PixelType;
  using InputPointDataContainer = typename InputMeshType::PointDataContainer;

  using OutputMeshType = TOutputMesh;
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using OutputEdgeCellType = typename OutputMeshType::EdgeCellType;
  using OutputPolygonCellType = typename OutputMeshType::PolygonCellType;
  using OutputQEType = typename OutputMeshType::QEType;
  using OutputPointIdentifier = typename OutputMeshType::PointIdentifier;
  using OutputPointType = typename OutputMeshType::PointType;
  using OutputVectorType = typename OutputPointType::VectorType;
  using OutputCoordType = typename OutputPointType::CoordRepType;
  using OutputPointsContainer = typename OutputMeshType::PointsContainer;
  using OutputPointsContainerPointer = typename OutputMeshType::PointsContainerPointer;
  using OutputPointsContainerIterator = typename OutputMeshType::PointsContainerIterator;
  using OutputCellsContainerPointer = typename OutputMeshType::CellsContainerPointer;
  using OutputCellsContainerIterator = typename OutputMeshType::CellsContainerIterator;
  using OutputPointDataContainer = typename OutputMeshType::PointDataContainer;
  using OutputPointDataContainerPointer = typename OutputMeshType::PointDataContainerPointer;
  using OutputPixelType = typename OutputMeshType::PixelType;

  static constexpr unsigned int PointDimension = OutputMeshType::PointDimension;

  /** The smoothing filter will run iteratively until reaching this maximum
   * number of iterations. Emprical observartions indicate that ten iterations
   * are enough for typical deformation fields, but of course this would depend
   * on the process that you used for generating your deformation field.
   */
  itkSetMacro( MaximumNumberOfIterations, unsigned long );
  itkGetMacro( MaximumNumberOfIterations, unsigned long );

  /** Factor that controls the degree of Smoothing. Large values of Lambda
   * result is stronger smoothing.  The Lambda factor is used to compute the
   * weights of the smoothing kernel as
   *
   * \f$
   * \frac{ \exp( \frac{-1}{2 \lambda} }{ 1 + \abs{ N_i } \exp( \frac{-1}{2 \lambda} }
   * \f$
   *
   * where \f$ N_i \f$ is the number of neighbor nodes around node \f$ i \f$.
   *
   * The default value of Lambda is 1.0.
   *
   * The filter assumes that the neighbor nodes of any given nodes are located
   * at similar distances, and therefore uses the same weight for each one of
   * the neighbor values when computing their weighted average.
   *
   */
  itkSetMacro( Lambda, double );
  itkGetMacro( Lambda, double );

protected:
  QuadEdgeMeshScalarPixelValuesSmoothingFilter();
  ~QuadEdgeMeshScalarPixelValuesSmoothingFilter();

  void
  GenerateData() override;

private:
  unsigned long m_MaximumNumberOfIterations;
  double        m_Lambda;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkQuadEdgeMeshScalarPixelValuesSmoothingFilter.hxx"
#endif

#endif
