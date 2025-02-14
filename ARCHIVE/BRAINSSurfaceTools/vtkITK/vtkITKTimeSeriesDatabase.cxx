/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesReader.cxx $
  Date:      $Date: 2008-04-03 07:35:41 -0500 (Thu, 03 Apr 2008) $
  Version:   $Revision: 6383 $

==========================================================================*/
#include "vtkITKTimeSeriesDatabase.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedLongArray.h>

vtkStandardNewMacro( vtkITKTimeSeriesDatabase );
int
vtkITKTimeSeriesDatabase::RequestInformation( vtkInformation *        vtkNotUsed( request ),
                                              vtkInformationVector ** vtkNotUsed( inputVector ),
                                              vtkInformationVector *  outputVector )
{
  // get the info objects
  vtkInformation * outInfo = outputVector->GetInformationObject( 0 );

  double spacing[3];
  double origin[3];
  for ( int i = 0; i < 3; i++ )
  {
    spacing[i] = this->m_Filter->GetOutputSpacing()[i];
    origin[i] = this->m_Filter->GetOutputOrigin()[i];
  }
  outInfo->Set( vtkDataObject::SPACING(), spacing, 3 );
  outInfo->Set( vtkDataObject::ORIGIN(), origin, 3 );
  int                                     extent[6];
  SourceType::OutputImageType::RegionType region = m_Filter->GetOutputRegion();
  extent[0] = region.GetIndex()[0];
  extent[1] = region.GetIndex()[0] + region.GetSize()[0] - 1;
  extent[2] = region.GetIndex()[1];
  extent[3] = region.GetIndex()[1] + region.GetSize()[1] - 1;
  extent[4] = region.GetIndex()[2];
  extent[5] = region.GetIndex()[2] + region.GetSize()[2] - 1;
  outInfo->Set( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6 );
  vtkDataObject::SetPointDataActiveScalarInfo( outInfo, VTK_SHORT, 1 );
  return 1;
};


  // defined in the subclasses
#if ( VTK_MAJOR_VERSION <= 5 )
void
vtkITKTimeSeriesDatabase::ExecuteData( vtkDataObject * output )
{
  vtkImageData * data = vtkImageData::SafeDownCast( output );
  data->SetExtent( 0, 0, 0, 0, 0, 0 );
  data->AllocateScalars();
  data->SetExtent( data->GetWholeExtent() );
#else
void
vtkITKTimeSeriesDatabase::ExecuteDataWithInformation( vtkDataObject * output, vtkInformation * outInfo )
{
  this->AllocateOutputData( output, outInfo );
#endif
  itk::ImportImageContainer< itk::SizeValueType, OutputImagePixelType >::Pointer PixelContainerShort;
  PixelContainerShort = this->m_Filter->GetOutput()->GetPixelContainer();
  void * ptr = static_cast< void * >( PixelContainerShort->GetBufferPointer() );
  vtkUnsignedLongArray::SafeDownCast( vtkImageData::SafeDownCast( output )->GetPointData()->GetScalars() )
    ->SetVoidArray( ptr, PixelContainerShort->Size(), 0, vtkDataArrayTemplate< unsigned long >::VTK_DATA_ARRAY_DELETE );
  PixelContainerShort->ContainerManageMemoryOff();
};
