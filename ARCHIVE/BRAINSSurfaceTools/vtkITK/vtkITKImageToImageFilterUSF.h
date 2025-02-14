/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKImageToImageFilterUSF.h $
  Date:      $Date: 2006-12-21 07:21:52 -0500 (Thu, 21 Dec 2006) $
  Version:   $Revision: 1900 $

==========================================================================*/

#ifndef __vtkITKImageToImageFilterUSF
#define __vtkITKImageToImageFilterUSF

#include "vtkITKImageToImageFilter.h"
#include "vtkImageAlgorithm.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"


class VTK_ITK_EXPORT vtkITKImageToImageFilterUSF : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro( vtkITKImageToImageFilterUSF, vtkITKImageToImageFilter );
  static vtkITKImageToImageFilterUSF *
  New()
  {
    return 0;
  };
  void
  PrintSelf( ostream & os, vtkIndent indent )
  {
    Superclass::PrintSelf( os, indent );
    os << m_Filter;
  };

protected:
  /// To/from ITK
  using InputImagePixelType = unsigned short;
  using OutputImagePixelType = float;
  using InputImageType = itk::Image< InputImagePixelType, 3 >;
  using OutputImageType = itk::Image< OutputImagePixelType, 3 >;

  using ImageImportType = itk::VTKImageImport< InputImageType >;
  using ImageExportType = itk::VTKImageExport< OutputImageType >;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  using GenericFilterType = itk::ImageToImageFilter< InputImageType, OutputImageType >;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilterUSF( GenericFilterType * filter )
  {
    /// Need an import, export, and a ITK pipeline
    m_Filter = filter;
    this->itkImporter = ImageImportType::New();
    this->itkExporter = ImageExportType::New();
    ConnectPipelines( this->vtkExporter, this->itkImporter );
    ConnectPipelines( this->itkExporter, this->vtkImporter );
    this->LinkITKProgressToVTKProgress( m_Filter );

    /// Set up the filter pipeline
    m_Filter->SetInput( this->itkImporter->GetOutput() );
    this->itkExporter->SetInput( m_Filter->GetOutput() );
    this->vtkCast->SetOutputScalarTypeToUnsignedShort();
  };

  ~vtkITKImageToImageFilterUSF(){};

private:
  vtkITKImageToImageFilterUSF( const vtkITKImageToImageFilterUSF & ); /// Not implemented.
  void
  operator=( const vtkITKImageToImageFilterUSF & ); /// Not implemented.
};

#endif
