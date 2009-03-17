/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHessian3DEigenAnalysisImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/03/17 17:07:42 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkHessian3DEigenAnalysisImageFilter_cxx
#define _itkHessian3DEigenAnalysisImageFilter_cxx

#include "itkHessian3DEigenAnalysisImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"
#include "itkSymmetricSecondRankTensor.h"


namespace itk
{


/**
 * Constructor.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::Hessian3DEigenAnalysisImageFilter() {
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfRequiredOutputs( 2 );
  this->SetNthOutput( 0, this->MakeOutput( 0 ) );
  this->SetNthOutput( 1, this->MakeOutput( 1 ) );

}


/**
 * Get the eigen values.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
typename Hessian3DEigenAnalysisImageFilter<TInputImage,TEigenValueImage,TEigenVectorImage>::EigenValueImageType*
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::GetEigenValues( void ) {
  return dynamic_cast<EigenValueImageType*>( this->GetOutput( 0 ) );
}


/**
 * Get the eigen vectors.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
typename Hessian3DEigenAnalysisImageFilter<TInputImage,TEigenValueImage,TEigenVectorImage>::EigenVectorImageType*
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::GetEigenVectors( void ) {
  EigenVectorImageType *eigenVector = dynamic_cast<EigenVectorImageType *>(
    this->GetOutput( 1 ) );

  return eigenVector;
}


/**
 * Make output types.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
DataObject::Pointer
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::MakeOutput(unsigned int idx) {
  DataObject::Pointer output;
  switch( idx ) {
    case 0:
      output = (EigenValueImageType::New()).GetPointer();
      break;
    case 1:
      output = (EigenVectorImageType::New()).GetPointer();
      break;
  }
  return output.GetPointer();
}


/**
 * Generate data.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
void
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::GenerateData() {

  typename TInputImage::ConstPointer inputPtr(
    dynamic_cast<const TInputImage*>(
    (ProcessObject::GetInput(0))));

  EigenValueImagePointer eigenValueOutputPtr = this->GetEigenValues();
  //EigenVectorImagePointer eigenVectorOutputPtr = this->GetEigenVectors();

  eigenValueOutputPtr->SetBufferedRegion( inputPtr->GetBufferedRegion() );
  //eigenVectorOutputPtr->SetBufferedRegion( inputPtr->GetBufferedRegion() );

  eigenValueOutputPtr->Allocate();
  //eigenVectorOutputPtr->Allocate();

  EigenValueImageRegionType region = eigenValueOutputPtr->GetRequestedRegion();

  ImageRegionConstIteratorWithIndex<TInputImage> inputIt( inputPtr, region );

  ImageRegionIteratorWithIndex<EigenValueImageType>  eigenValueIt(eigenValueOutputPtr, region);
  //ImageRegionIteratorWithIndex<EigenVectorImageType> eigenVectorIt(eigenVectorOutputPtr, region);

  EigenValueImagePixelType nullEigenValue;
  nullEigenValue.Fill( 0.0 );

  EigenVectorImagePixelType nullEigenVector;
  nullEigenVector.Fill( 0.0 );

  // Support progress methods/callbacks.
  ProgressReporter progress(this, 0, region.GetNumberOfPixels());

  inputIt.GoToBegin();
  eigenValueIt.GoToBegin();
  //eigenVectorIt.GoToBegin();

  while ( !inputIt.IsAtEnd() ) {
    
    TInputImage::PixelType matrix = inputIt.Get();

    EigenValueImagePixelType eigenValues;
    EigenVectorImagePixelType eigenVectors;

    matrix.ComputeEigenAnalysis(eigenValues, eigenVectors);

    eigenValueIt.Set( eigenValues );
    //eigenVectorIt.Set( eigenVectors );

    ++inputIt;
    ++eigenValueIt;
    //++eigenVectorIt;

    progress.CompletedPixel();
  }

}

} // end namespace itk

#endif // _itkHessian3DEigenAnalysisImageFilter_cxx