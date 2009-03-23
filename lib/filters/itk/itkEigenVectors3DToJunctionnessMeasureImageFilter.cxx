/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/03/23 00:00:50 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx
#define _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx

#include "itkEigenVectors3DToJunctionnessMeasureImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

namespace itk {

template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::EigenVectors3DToJunctionnessImageFilter() {
  m_Radius.Fill(1);
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
void
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::SetEigenVectorInput(typename EigenVectorImageType::Pointer input) {
  this->SetNthInput(0, input);
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
typename EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>::EigenVectorImageType::Pointer
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::GetEigenVectorInput() {
  typename EigenVectorImageType::Pointer eigenVectorPtr =
    static_cast< TEigenVectorImage * >( this->ProcessObject::GetInput(0) );
  return eigenVectorPtr;
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
void
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::SetVesselnessInput(typename VesselnessImageType::Pointer input) {
  this->SetNthInput(1, input);
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
typename EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>::VesselnessImageType::Pointer
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::GetVesselnessInput() {
  typename VesselnessImageType::Pointer vesselnessPtr =
    static_cast< TVesselnessImage * >( this->ProcessObject::GetInput(1) );
  return vesselnessPtr;
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
void
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError) {
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // Get pointers to the input and output.
  EigenVectorImageType::Pointer eigenVectorPtr =
    this->GetEigenVectorInput();
  VesselnessImageType::Pointer vesselnessPtr =
    this->GetVesselnessInput();
  typename OutputImageType::Pointer outputPtr = this->GetOutput();

  if ( !eigenVectorPtr || !vesselnessPtr || !outputPtr ) {
    return;
  }

  // Get a copy of the input requested region (should equal the output
  // requested region)
  EigenVectorImageRegionType  eigenVectorRequestedRegion;
  eigenVectorRequestedRegion = eigenVectorPtr->GetRequestedRegion();

  // Pad the input requested region by the operator radius.
  eigenVectorRequestedRegion.PadByRadius( m_Radius );

  // Crop the input requested region at the input's largest possible region.
  if ( eigenVectorRequestedRegion.Crop(eigenVectorPtr->GetLargestPossibleRegion()) ) {
    eigenVectorPtr->SetRequestedRegion( eigenVectorRequestedRegion );
    vesselnessPtr->SetRequestedRegion( eigenVectorRequestedRegion );
    return;
  } else {
    // Couldn't crop the region (requested region is outside the largest
    // possible region). Throw an exception.

    // Store what we tried to request (prior to trying to crop)
    eigenVectorPtr->SetRequestedRegion( eigenVectorRequestedRegion );
    vesselnessPtr->SetRequestedRegion( eigenVectorRequestedRegion );

    // Build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(eigenVectorPtr);
    throw e;
  }
}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
void
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId) {
  ZeroFluxNeumannBoundaryCondition<EigenVectorImageType> eigenVectorNbc;
  ZeroFluxNeumannBoundaryCondition<VesselnessImageType> vesselnessNbc;

  ConstNeighborhoodIterator<EigenVectorImageType> eigenVectorNbrIt;
  ConstNeighborhoodIterator<VesselnessImageType> vesselnessNbrIt;
  ImageRegionIterator<OutputImageType> outputIt;

  typename OutputImageType::Pointer output = this->GetOutput();
  typename EigenVectorImageType::ConstPointer eigenVectorInput = 
    this->GetEigenVectorInput();
  typename VesselnessImageType::ConstPointer  vesselnessInput =
    this->GetVesselnessInput();

  // Find the data set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<EigenVectorImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<EigenVectorImageType> bC;
  faceList = bC(eigenVectorInput, outputRegionForThread, m_Radius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<EigenVectorImageType>::FaceListType::iterator fit;

  // Get image origin and pixel spacing
  itk::Vector<double> spacing = eigenVectorInput->GetSpacing();
  EigenVectorImageType::PointType origin = eigenVectorInput->GetOrigin();

  // Suppor progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Process each of the boundary faces. These are N-d regions which border
  // the edge of the buffer.
  for (fit = faceList.begin(); fit != faceList.end(); ++fit) {
    eigenVectorNbrIt = ConstNeighborhoodIterator<EigenVectorImageType>(m_Radius,
      eigenVectorInput, *fit);
    vesselnessNbrIt = ConstNeighborhoodIterator<VesselnessImageType>(m_Radius,
      vesselnessInput, *fit);

    unsigned int neighborhoodSize = eigenVectorNbrIt.Size();
    outputIt = ImageRegionIterator<OutputImageType>(output, *fit);
    eigenVectorNbrIt.OverrideBoundaryCondition(&eigenVectorNbc);
    vesselnessNbrIt.OverrideBoundaryCondition(&vesselnessNbc);
    eigenVectorNbrIt.GoToBegin();
    vesselnessNbrIt.GoToBegin();

    NumericTraits<OutputPixelType>::RealType sum;

    while (!eigenVectorNbrIt.IsAtEnd() ) {

      sum = NumericTraits<NumericTraits<OutputPixelType>::RealType>::Zero;
      for (int i = 0; i < neighborhoodSize; ++i) {
        // Get the eigen vector matrix.
        EigenVectorPixelType eigenVectorMatrix = eigenVectorNbrIt.GetPixel(i);

        // Compute spatial position of this neighbor pixel coordinates.
        ConstNeighborhoodIterator<EigenVectorImageType>::IndexType nbrIndex
          = eigenVectorNbrIt.GetIndex(i);
        NumericTraits<OutputPixelType>::RealType pixelPosition[EigenVectorImageDimension];
        for (int j = 0; j < EigenVectorImageDimension; ++j) {
          pixelPosition[j] = (spacing[j]*nbrIndex[j]) + origin[j];
        }

        // Compute vector from center of the pixel to this neighbor pixel.
        
        // Take the absolute dot product of the eigen vector corresponding to the least
        // principal curvature direction and the template direction

        // Weight the absolute dot product with the vesselness measure.


        sum += static_cast<OutputPixelType>(vesselnessNbrIt.GetPixel(i));
      }

      // Set value here.
      outputIt.Set(static_cast<OutputPixelType>(sum));

      ++eigenVectorNbrIt;
      ++vesselnessNbrIt;
      ++outputIt;
      progress.CompletedPixel();
    }
  }

}

} // end namespace itk


#endif // _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx
