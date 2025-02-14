/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKIslandMath.cxx $
  Date:      $Date: 2006-12-21 07:21:52 -0500 (Thu, 21 Dec 2006) $
  Version:   $Revision: 1900 $

==========================================================================*/

#include "vtkITKIslandMath.h"
#include "vtkObjectFactory.h"

#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkAlgorithm.h"
#include <vtkVersion.h>

#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkCommand.h"

vtkStandardNewMacro( vtkITKIslandMath );

vtkITKIslandMath::vtkITKIslandMath()
{
  this->FullyConnected = 0;
  this->SliceBySlice = 0;
  this->MinimumSize = 0;
#if ( VTK_MAJOR_VERSION <= 5 )
  this->MaximumSize = VTK_LARGE_ID;
#else
  this->MaximumSize = VTK_ID_MAX;
#endif
  this->NumberOfIslands = 0;
  this->OriginalNumberOfIslands = 0;
}

vtkITKIslandMath::~vtkITKIslandMath() {}

void
vtkITKIslandMath::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "FullyConnected: " << FullyConnected << std::endl;
  os << indent << "SliceBySlice: " << SliceBySlice << std::endl;
  os << indent << "MinimumSize: " << MinimumSize << std::endl;
  os << indent << "MaximumSize: " << MaximumSize << std::endl;
  os << indent << "NumberOfIslands: " << NumberOfIslands << std::endl;
  os << indent << "OriginalNumberOfIslands: " << OriginalNumberOfIslands << std::endl;
}

// Note: local function not method - conforms to signature in itkCommand.h
void
vtkITKIslandMathHandleProgressEvent( itk::Object * caller, const itk::EventObject & vtkNotUsed( eventObject ),
                                     void * clientdata )
{
  itk::ProcessObject * itkFilter = dynamic_cast< itk::ProcessObject * >( caller );
  vtkAlgorithm *       vtkFilter = reinterpret_cast< vtkAlgorithm * >( clientdata );
  if ( itkFilter && vtkFilter )
  {
    vtkFilter->UpdateProgress( itkFilter->GetProgress() );
  }
};

template < typename T >
void
vtkITKIslandMathExecute( vtkITKIslandMath * self, vtkImageData * input, vtkImageData * vtkNotUsed( output ), T * inPtr,
                         T * outPtr )
{

  int dims[3];
  input->GetDimensions( dims );
  double spacing[3];
  input->GetSpacing( spacing );

  // Wrap scalars into an ITK image
  // - mostly rely on defaults for spacing, origin etc for this filter
  using ImageType = itk::Image< T, 3 >;
  typename ImageType::Pointer    inImage = ImageType::New();
  typename ImageType::RegionType region;
  typename ImageType::IndexType  index;
  typename ImageType::SizeType   size;

  inImage->GetPixelContainer()->SetImportPointer( inPtr, dims[0] * dims[1] * dims[2], false );
  index[0] = index[1] = index[2] = 0;
  region.SetIndex( index );
  size[0] = dims[0];
  size[1] = dims[1];
  size[2] = dims[2];
  region.SetSize( size );
  inImage->SetLargestPossibleRegion( region );
  inImage->SetBufferedRegion( region );
  inImage->SetSpacing( spacing );

  // set up the progress callback
  itk::CStyleCommand::Pointer progressCommand = itk::CStyleCommand::New();
  progressCommand->SetClientData( static_cast< void * >( self ) );
  progressCommand->SetCallback( vtkITKIslandMathHandleProgressEvent );


  // Calculate the island operation
  // ccfilter - identifies the islands
  // relabel - sorts them by size
  using ConnectedComponentType = itk::ConnectedComponentImageFilter< ImageType, ImageType >;
  typename ConnectedComponentType::Pointer ccfilter = ConnectedComponentType::New();
  using RelabelComponentType = itk::RelabelComponentImageFilter< ImageType, ImageType >;
  typename RelabelComponentType::Pointer relabel = RelabelComponentType::New();

  ccfilter->AddObserver( itk::ProgressEvent(), progressCommand );
  relabel->AddObserver( itk::ProgressEvent(), progressCommand );

  ccfilter->SetFullyConnected( self->GetFullyConnected() );
  ccfilter->SetInput( inImage );
  relabel->SetInput( ccfilter->GetOutput() );
  relabel->SetMinimumObjectSize( self->GetMinimumSize() );
  relabel->Update();
  self->SetNumberOfIslands( relabel->GetNumberOfObjects() );
  self->SetOriginalNumberOfIslands( relabel->GetOriginalNumberOfObjects() );

  // Copy to the output
  memcpy( outPtr,
          relabel->GetOutput()->GetBufferPointer(),
          relabel->GetOutput()->GetBufferedRegion().GetNumberOfPixels() * sizeof( T ) );
}


//
//
//
void
vtkITKIslandMath::SimpleExecute( vtkImageData * input, vtkImageData * output )
{
  vtkDebugMacro( << "Executing Island Math" );

  //
  // Initialize and check input
  //
  vtkPointData * pd = input->GetPointData();
  pd = input->GetPointData();
  if ( pd == nullptr )
  {
    vtkErrorMacro( << "PointData is NULL" );
    return;
  }
  vtkDataArray * inScalars = pd->GetScalars();
  if ( inScalars == nullptr )
  {
    vtkErrorMacro( << "Scalars must be defined for island math" );
    return;
  }

  if ( inScalars->GetNumberOfComponents() == 1 )
  {

////////// These types are not defined in itk ////////////
#undef VTK_TYPE_USE_LONG_LONG
#undef VTK_TYPE_USE___INT64

#define CALL                                                                                                           \
  vtkITKIslandMathExecute( this, input, output, static_cast< VTK_TT * >( inPtr ), static_cast< VTK_TT * >( outPtr ) );

    void * inPtr = input->GetScalarPointer();
    void * outPtr = output->GetScalarPointer();

    switch ( inScalars->GetDataType() )
    {
      vtkTemplateMacroCase( VTK_LONG, long, CALL );
      vtkTemplateMacroCase( VTK_UNSIGNED_LONG, unsigned long, CALL );
      vtkTemplateMacroCase( VTK_INT, int, CALL );
      vtkTemplateMacroCase( VTK_UNSIGNED_INT, unsigned int, CALL );
      vtkTemplateMacroCase( VTK_SHORT, short, CALL );
      vtkTemplateMacroCase( VTK_UNSIGNED_SHORT, unsigned short, CALL );
      vtkTemplateMacroCase( VTK_CHAR, char, CALL );
      vtkTemplateMacroCase( VTK_SIGNED_CHAR, signed char, CALL );
      vtkTemplateMacroCase( VTK_UNSIGNED_CHAR, unsigned char, CALL );
      default:
      {
        vtkErrorMacro( << "Incompatible data type for this version of ITK." );
      }
    } // switch
  }
  else
  {
    vtkErrorMacro( << "Only single component images supported." );
  }
}
