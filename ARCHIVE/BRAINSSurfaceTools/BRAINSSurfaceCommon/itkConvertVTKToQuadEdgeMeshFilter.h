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
#ifndef __itkConvertVTKToQuadEdgeMeshFilter_h
#define __itkConvertVTKToQuadEdgeMeshFilter_h

#include "itkMesh.h"
#include "itkMeshSource.h"
#include "itkTriangleCell.h"
#include "itkMapContainer.h"

#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkIdList.h"

namespace itk
{
/** \class ConvertVTKToQuadEdgeMeshFilter
 * \brief
 * take a vtkPolyData as input and create an itkMesh.
 *
 * Caveat: itkConvertVTKToQuadEdgeMeshFilter can only convert triangle meshes.
 *         Use vtkTriangleFilter to convert your mesh to a triangle mesh.
 */
template < typename TOutputMesh >
class ConvertVTKToQuadEdgeMeshFilter : public MeshSource< TOutputMesh >
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( ConvertVTKToQuadEdgeMeshFilter );

  /** Standard "Self" type alias. */
  using Self = ConvertVTKToQuadEdgeMeshFilter;
  using Superclass = MeshSource< TOutputMesh >;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( ConvertVTKToQuadEdgeMeshFilter, MeshSource );

  /** Hold on to the type information specified by the template parameters. */
  using OutputMeshType = TOutputMesh;
  using PointType = typename OutputMeshType::PointType;
  using PixelType = typename OutputMeshType::PixelType;

  /** Some convenient type alias. */
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using CellTraits = typename OutputMeshType::CellTraits;
  using CellIdentifier = typename OutputMeshType::CellIdentifier;
  using CellType = typename OutputMeshType::CellType;
  using CellAutoPointer = typename OutputMeshType::CellAutoPointer;
  using PointIdentifier = typename OutputMeshType::PointIdentifier;
  using PointIdListType = typename OutputMeshType::PointIdList;
  using PointIdIterator = typename CellTraits::PointIdIterator;

  /** Define the triangular cell types which form the surface  */
  using TriangleCellType = TriangleCell< CellType >;

  typedef typename TriangleCellType::SelfAutoPointer TriangleCellAutoPointer;

  using IndexPairType = std::pair< unsigned long, unsigned long >;
  using PointMapType = MapContainer< IndexPairType, unsigned long >;
  using VectorType = typename PointType::VectorType;

  /* set/get the input polydata */
  void
  SetPolyData( vtkPolyData * );

  vtkPolyData *
  GetPolyData();

protected:
  ConvertVTKToQuadEdgeMeshFilter();
  ~ConvertVTKToQuadEdgeMeshFilter(){};

  /** convert the polydata */
  void
  GenerateData() override;

private:
  vtkPolyData * m_inputPolyData;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkConvertVTKToQuadEdgeMeshFilter.hxx"
#endif

#endif // _itkConvertVTKToQuadEdgeMeshFilter_h
