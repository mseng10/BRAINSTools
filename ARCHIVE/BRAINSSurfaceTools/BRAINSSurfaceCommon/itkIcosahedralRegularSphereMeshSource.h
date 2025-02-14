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
#ifndef __itkIcosahedralRegularSphereMeshSource_h
#define __itkIcosahedralRegularSphereMeshSource_h

#include "itkMesh.h"
#include "itkMeshSource.h"
#include "itkTriangleCell.h"
#include "itkMapContainer.h"

namespace itk
{
/** \class IcosahedralRegularSphereMeshSource
 * \brief
 * Inputs are the center of the mesh, the scale (radius in each dimension) of
 * the mesh and a resolution parameter, which corresponds to the recursion
 * depth whlie creating a spherical triangle mesh.
 *
 * Don't use recursion depths larger than 5, because mesh generation gets very slow.
 *
 * This class is largely base on the itkRegularSphereMeshSource class
 * contributed by Thomas Boettger. Division Medical and Biological Informatics,
 * German Cancer Research Center, Heidelberg.
 *
 */
template < typename TOutputMesh >
class IcosahedralRegularSphereMeshSource : public MeshSource< TOutputMesh >
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( IcosahedralRegularSphereMeshSource );

  /** Standard "Self" type alias. */
  using Self = IcosahedralRegularSphereMeshSource;
  using Superclass = itk::MeshSource< TOutputMesh >;
  using Pointer = itk::SmartPointer< Self >;
  using ConstPointer = itk::SmartPointer< const Self >;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( IcosahedralRegularSphereMeshSource, MeshSource );

  /** Hold on to the type information specified by the template parameters. */
  using OutputMeshType = TOutputMesh;
  using MeshTraits = typename OutputMeshType::MeshTraits;
  using PointType = typename OutputMeshType::PointType;
  using PixelType = typename MeshTraits::PixelType;

  /** Some convenient type alias. */
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using CellTraits = typename OutputMeshType::CellTraits;
  using PointsContainerPointer = typename OutputMeshType::PointsContainerPointer;
  using PointsContainer = typename OutputMeshType::PointsContainer;

  /** Define the triangular cell types which form the surface  */
  using CellInterfaceType = itk::CellInterface< PixelType, CellTraits >;
  using TriCellType = itk::TriangleCell< CellInterfaceType >;
  using TriCellAutoPointer = typename TriCellType::SelfAutoPointer;
  using CellAutoPointer = typename TriCellType::CellAutoPointer;

  using IndexPairType = std::pair< unsigned long, unsigned long >;
  using PointMapType = itk::MapContainer< IndexPairType, unsigned long >;
  using VectorType = typename PointType::VectorType;

  /** Set the resolution level to be used for generating cells in the Sphere.
   *  High values of this parameter will produce sphere with more triangles. */
  itkSetMacro( Resolution, unsigned int );
  itkGetConstMacro( Resolution, unsigned int );

  /** Set/Get Coordinates of the Sphere center. */
  itkSetMacro( Center, PointType );
  itkGetConstMacro( Center, PointType );

  /** Set/Get scales of the Sphere. This is a vector of values that can
   * actually be used for generating ellipsoids aligned with the coordinate
   * axis. */
  itkSetMacro( Scale, VectorType );
  itkGetConstMacro( Scale, VectorType );

protected:
  IcosahedralRegularSphereMeshSource();
  ~IcosahedralRegularSphereMeshSource() {}

  void
  PrintSelf( std::ostream & os, itk::Indent indent ) const override;

  void
  GenerateData() override;

  PointType
  Divide( const PointType & p1, const PointType & p2 ) const;

  void
  AddCell( OutputMeshType * mesh, const unsigned long * pointIds, unsigned long idx );

  /** model center */
  PointType m_Center;

  /** models resolution */
  unsigned int m_Resolution;

  /** model scales */
  VectorType m_Scale;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkIcosahedralRegularSphereMeshSource.hxx"
#endif

#endif // _itkIcosahedralRegularSphereMeshSource_h
