/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVTKImageToImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/03/16 13:35:43 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkVTKImageToImageFilter_h
#define __itkVTKImageToImageFilter_h

#include "itkVTKImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageData.h"

#ifndef vtkFloatingPointType
#  define vtkFloatingPointType float
#endif

namespace itk
{
/** \class VTKImageToImageFilter
 * \brief Converts a VTK image into an ITK image and plugs a
 *  vtk data pipeline to an ITK datapipeline.
 *
 *  This class puts together an itkVTKImageImporter and a vtkImageExporter.
 *  It takes care of the details related to the connection of ITK and VTK
 *  pipelines. The User will perceive this filter as an adaptor to which
 *  a vtkImage can be plugged as input and an itk::Image is produced as
 *  output.
 *
 * \ingroup   ImageFilters
 */
template < typename TOutputImage >
class ITK_EXPORT VTKImageToImageFilter : public ProcessObject
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN( VTKImageToImageFilter );

  /** Standard class type alias. */
  using Self = VTKImageToImageFilter;
  using Superclass = ProcessObject;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer< const Self >;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( VTKImageToImageFilter, ProcessObject );

  /** Some type alias. */
  using OutputImageType = TOutputImage;
  using OutputImagePointer = typename OutputImageType::ConstPointer;
  using ImporterFilterType = VTKImageImport< OutputImageType >;
  using ImporterFilterPointer = typename ImporterFilterType::Pointer;

  /** Get the output in the form of a vtkImage.
      This call is delegated to the internal vtkImageImporter filter  */
  const OutputImageType *
  GetOutput() const;

  /** Set the input in the form of a vtkImageData */
  void
  SetInputData( vtkImageData * );

  /** Return the internal VTK image exporter filter.
      This is intended to facilitate users the access
      to methods in the exporter */
  vtkImageExport *
  GetExporter() const;

  /** Return the internal ITK image importer filter.
      This is intended to facilitate users the access
      to methods in the importer */
  ImporterFilterType *
  GetImporter() const;

  /** This call delegate the update to the importer */
  void
  Update() override;

protected:
  VTKImageToImageFilter();
  virtual ~VTKImageToImageFilter();

private:
  ImporterFilterPointer m_Importer;
  vtkImageExport *      m_Exporter;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkVTKImageToImageFilter.hxx"
#endif

#endif
