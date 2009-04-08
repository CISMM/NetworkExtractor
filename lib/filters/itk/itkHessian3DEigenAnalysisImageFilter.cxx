/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHessian3DEigenAnalysisImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/04/08 14:29:52 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkHessian3DEigenAnalysisImageFilter_cxx
#define _itkHessian3DEigenAnalysisImageFilter_cxx

#include "itkHessian3DEigenAnalysisImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
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
typename Hessian3DEigenAnalysisImageFilter<TInputImage,TEigenValueImage,TEigenVectorImage>::EigenValueImageType *
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::GetEigenValues( void ) {
  EigenValueImageType* eigenValue;
  try {
    eigenValue = dynamic_cast<EigenValueImageType *>(
      this->ProcessObject::GetOutput( 0 ));
  } catch (std::bad_cast) {
    std::cout << "Hessian3DEigenAnalysisImageFilter::GetEigenValues: Cannot dynamic_cast." << std::endl;
  }

  return eigenValue;
}


/**
 * Get the eigen vectors.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
typename Hessian3DEigenAnalysisImageFilter<TInputImage,TEigenValueImage,TEigenVectorImage>::EigenVectorImageType *
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::GetEigenVectors( void ) {
  EigenVectorImageType* eigenVector;
  try {
    eigenVector = dynamic_cast<EigenVectorImageType *>(
      this->ProcessObject::GetOutput( 1 ));
  } catch (std::bad_cast) {
    std::cout << "Hessian3DEigenAnalysisImageFilter::GetEigenVectors: Cannot dynamic_cast" << std::endl;
  }

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
      output = static_cast<DataObject*>(EigenValueImageType::New().GetPointer());
      break;
    case 1:
      output = static_cast<DataObject*>(EigenVectorImageType::New().GetPointer());
      break;
  }
  return output;
}


/** Override superclass memory allocation routine. */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
void
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::AllocateOutputs() {

  // Allocate the output memory
  EigenValueImagePointer eigenValueOutputPtr = this->GetEigenValues();
  eigenValueOutputPtr->SetBufferedRegion(eigenValueOutputPtr->GetRequestedRegion());
  eigenValueOutputPtr->Allocate();

  EigenVectorImagePointer eigenVectorOutputPtr = this->GetEigenVectors();
  eigenVectorOutputPtr->SetBufferedRegion(eigenVectorOutputPtr->GetRequestedRegion());
  eigenVectorOutputPtr->Allocate();
}


/**
 * Generate data.
 */
template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
void
Hessian3DEigenAnalysisImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>
::ThreadedGenerateData(const EigenValueImageRegionType& outputRegionForThread, int threadId) {

  typename TInputImage::ConstPointer inputPtr(
    dynamic_cast<const TInputImage*>(
    (ProcessObject::GetInput(0))));

  EigenValueImagePointer eigenValueOutputPtr = this->GetEigenValues();
  EigenVectorImagePointer eigenVectorOutputPtr = this->GetEigenVectors();

  ImageRegionConstIteratorWithIndex<TInputImage> inputIt( inputPtr, outputRegionForThread );

  ImageRegionIterator<EigenValueImageType>  eigenValueIt(eigenValueOutputPtr, outputRegionForThread);
  ImageRegionIterator<EigenVectorImageType> eigenVectorIt(eigenVectorOutputPtr, outputRegionForThread);

  // Support progress methods/callbacks.
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  inputIt.GoToBegin();
  eigenValueIt.GoToBegin();
  eigenVectorIt.GoToBegin();

  EigenValueImagePixelType eigenValues;
  Matrix<EigenValueImagePixelType::ValueType, Self::ImageDimension, Self::ImageDimension> eigenVectorMatrix;
  EigenVectorImagePixelType eigenVector;

  while ( !inputIt.IsAtEnd() ) {
    
    TInputImage::PixelType matrix = inputIt.Get();

    // By default, matrix.ComputeEigenAnalysis sorts eigenvalues and 
    // eigenvectors by eigenvalue in ascending order.
    matrix.ComputeEigenAnalysis(eigenValues, eigenVectorMatrix);

    // Save vector associated with largest eigen value (least negative value) here.
    // Vectors are stored in rows of the matrices.
    for (int i = 0; i < Self::ImageDimension; i++) {
      eigenVector[i] = eigenVectorMatrix[2][i];
    }

    eigenValueIt.Set( eigenValues );
    eigenVectorIt.Set( eigenVector );

    ++inputIt;
    ++eigenValueIt;
    ++eigenVectorIt;

    progress.CompletedPixel();
  }

}

} // end namespace itk

#endif // _itkHessian3DEigenAnalysisImageFilter_cxx