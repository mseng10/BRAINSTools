/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderFile.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataArrayTemplate.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

// ITK includes
#include <itkImageFileReader.h>
#include <itkOrientImageFilter.h>

vtkStandardNewMacro( vtkITKArchetypeImageSeriesVectorReaderFile );

namespace
{

template < typename T >
vtkDataArrayTemplate< T > *
DownCast( vtkAbstractArray * a )
{
#if VTK_MAJOR_VERSION <= 5
  return static_cast< vtkDataArrayTemplate< T > * >( a );
#else
  return vtkDataArrayTemplate< T >::FastDownCast( a );
#endif
}

}; // namespace

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::vtkITKArchetypeImageSeriesVectorReaderFile() {}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::~vtkITKArchetypeImageSeriesVectorReaderFile() {}

//----------------------------------------------------------------------------
void
vtkITKArchetypeImageSeriesVectorReaderFile::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "vtk ITK Archetype Image Series Vector Reader File\n";
}

//----------------------------------------------------------------------------
template < typename T >
void
vtkITKExecuteDataFromFileVector( vtkITKArchetypeImageSeriesVectorReaderFile * self, vtkImageData * data )
{
  using image2 = itk::VectorImage< T, 3 >;
  using FilterType = itk::ImageSource< image2 >;
  typename FilterType::Pointer filter;
  using ReaderType = itk::ImageFileReader< image2 >;
  typename ReaderType::Pointer reader2 = ReaderType::New();
  reader2->SetFileName( self->GetFileName( 0 ) );
  if ( self->GetUseNativeCoordinateOrientation() )
  {
    filter = reader2;
  }
  else
  {
    typename itk::OrientImageFilter< image2, image2 >::Pointer orient2 =
      itk::OrientImageFilter< image2, image2 >::New();
    orient2->SetDebug( self->GetDebug() );
    orient2->SetInput( reader2->GetOutput() );
    orient2->UseImageDirectionOn();
    orient2->SetDesiredCoordinateOrientation( self->GetDesiredCoordinateOrientation() );
    filter = orient2;
  }
  filter->UpdateLargestPossibleRegion();
  typename itk::ImportImageContainer< itk::SizeValueType, T >::Pointer PixelContainer2;
  PixelContainer2 = filter->GetOutput()->GetPixelContainer();
  void * ptr = static_cast< void * >( PixelContainer2->GetBufferPointer() );
  DownCast< T >( data->GetPointData()->GetScalars() )
    ->SetVoidArray( ptr, PixelContainer2->Size(), 0, vtkDataArrayTemplate< T >::VTK_DATA_ARRAY_DELETE );
  PixelContainer2->ContainerManageMemoryOff();
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
#if ( VTK_MAJOR_VERSION <= 5 )
void
vtkITKArchetypeImageSeriesVectorReaderFile::ExecuteData( vtkDataObject * output )
{
  if ( !this->Superclass::Archetype )
  {
    vtkErrorMacro( "An Archetype must be specified." );
    return;
  }

  vtkImageData * data = vtkImageData::SafeDownCast( output );
  // data->UpdateInformation();
  data->SetExtent( 0, 0, 0, 0, 0, 0 );
  data->AllocateScalars();
  data->SetExtent( data->GetWholeExtent() );
#else
void
vtkITKArchetypeImageSeriesVectorReaderFile::ExecuteDataWithInformation( vtkDataObject * output,
                                                                        vtkInformation * outInfo )
{
  if ( !this->Superclass::Archetype )
  {
    vtkErrorMacro( "An Archetype must be specified." );
    return;
  }
  vtkImageData * data = this->AllocateOutputData( output, outInfo );
#endif

  // If there is only one file in the series, just use an image file reader
  if ( this->FileNames.size() == 1 )
  {
    vtkDebugMacro( "ImageSeriesVectorReaderFile: only one file: " << this->FileNames[0].c_str() );
    vtkDebugMacro( "DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str() );
    switch ( this->OutputScalarType )
    {
      vtkTemplateMacroCase( VTK_DOUBLE, double, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_FLOAT, float, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_LONG, long, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_INT, int, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_SHORT, short, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase(
        VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_CHAR, char, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_SIGNED_CHAR, signed char, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      vtkTemplateMacroCase( VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromFileVector< VTK_TT >( this, data ) );
      default:
        vtkErrorMacro( << "UpdateFromFile: Unknown data type " << this->OutputScalarType );
    }
  }
  else
  {
    // ERROR - should have used the series reader
    vtkErrorMacro( "There is more than one file, use the VectorReaderSeries instead" );
  }
}


void
vtkITKArchetypeImageSeriesVectorReaderFile::ReadProgressCallback( itk::ProcessObject * obj, const itk::ProgressEvent &,
                                                                  void *               data )
{
  vtkITKArchetypeImageSeriesVectorReaderFile * me =
    reinterpret_cast< vtkITKArchetypeImageSeriesVectorReaderFile * >( data );
  me->Progress = obj->GetProgress();
  me->InvokeEvent( vtkCommand::ProgressEvent, &me->Progress );
}
