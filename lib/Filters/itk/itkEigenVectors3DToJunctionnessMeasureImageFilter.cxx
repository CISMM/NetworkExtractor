/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/22 22:55:53 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx
#define _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx

#include "itkEigenVectors3DToJunctionnessMeasureImageFilter.h"

#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkOffset.h>
#include <itkProgressReporter.h>

#include <itkVTKPolyDataWriter.h>

namespace itk {

template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::EigenVectors3DToJunctionnessImageFilter() {
  this->m_Radius = 1.0;
  this->m_VesselnessThreshold = 1.0;

  this->m_SphereSampleSource = SphereSourceType::New();
  this->m_SphereSampleSource->SetScale(1.0);
  this->m_SphereSampleSource->SetResolution(3);
  this->m_SphereSampleSource->Update();

  // Set up interpolators for the vesselness image and eigenvector image.
  this->m_VesselnessInterpolator = VesselnessInterpolatorType::New();
  this->m_EigenVectorInterpolator = EigenVectorInterpolatorType::New();

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
  typename EigenVectorImageType::Pointer eigenVectorPtr =
    this->GetEigenVectorInput();
  typename VesselnessImageType::Pointer vesselnessPtr =
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
  EigenVectorSizeType logicalRadius = this->GetLogicalRadius();
  eigenVectorRequestedRegion.PadByRadius( logicalRadius );

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

  EigenVectorImagePointer eigenVectorInput = this->GetEigenVectorInput();
  VesselnessImagePointer  vesselnessInput = this->GetVesselnessInput();
  OutputImagePointer output = this->GetOutput();

  // Set the input for the vesselness interpolator.
  this->m_EigenVectorInterpolator->SetInputImage(eigenVectorInput);
  this->m_VesselnessInterpolator->SetInputImage(vesselnessInput);

  // Reduce the iterator region by the logical size of the junction
  // filter radius so we don't read outside the image boundaries. The
  // junctionness measure will thus not be computed on the image boundaries.
  EigenVectorSizeType logicalRadius = this->GetLogicalRadius();
  EigenVectorSizeType negativeOne;
  negativeOne.Fill(-1.0);

  // Causes an underflow in unsigned representations, but that is okay
  // because when this value is added to the whole region, an overflow
  // will occur, bringing the region boundaries to sane values.
  logicalRadius *= negativeOne;

  EigenVectorImageRegionType wholeRegion 
    = eigenVectorInput->GetLargestPossibleRegion();
  wholeRegion.PadByRadius(logicalRadius);
  OutputImageRegionType croppedOutputRegionForThread = outputRegionForThread;
  croppedOutputRegionForThread.Crop(wholeRegion);

  EigenVectorConstIteratorWithIndex eigenVectorIt( eigenVectorInput, outputRegionForThread );
  VesselnessConstIteratorWithIndex  vesselnessIt( vesselnessInput, outputRegionForThread );
  OutputImageIteratorWithIndex      outputIt( output, outputRegionForThread );

  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Iterate over all the pixels, computing the junctionness measure.
  OutputRealType       sum;
  for (eigenVectorIt.GoToBegin(), vesselnessIt.GoToBegin(), outputIt.GoToBegin();
    !eigenVectorIt.IsAtEnd(); ++eigenVectorIt, ++vesselnessIt, ++outputIt) {

    sum = NumericTraits<OutputRealType>::Zero;

    // Make she we are within legal bounds
    typename EigenVectorConstIteratorWithIndex::IndexType index = eigenVectorIt.GetIndex();

    // Skip this voxel if the vesselness value is below a threshold value or if we are
    // outside the valid region.
    VesselnessPixelType voxelValue = vesselnessIt.Get();
    if (voxelValue < this->m_VesselnessThreshold || !croppedOutputRegionForThread.IsInside(index)) {
      outputIt.Set(static_cast<OutputPixelType>(sum));
      progress.CompletedPixel();
      continue;
    }
  
    // Get the spatial location of the voxel. We'll use this to shift the 
    // sample points on the sampling sphere.
    typename EigenVectorImageType::PointType voxelPoint;
    eigenVectorInput->TransformIndexToPhysicalPoint(index, voxelPoint);
    
    // Iterate over directional samples
    MeshType* mesh = this->m_SphereSampleSource->GetOutput();
    for (unsigned long i = 0; i < mesh->GetNumberOfPoints(); i++) {
      SphereSourceType::PointType point;
      mesh->GetPoint(i, &point);
      EigenVectorPixelType sampleDirection = EigenVectorPixelType();

      for (int j = 0; j < EigenVectorPixelType::Length; j++) {
        // Store the vector of the sample.
        sampleDirection[j] = point[j];

        // Scale sample point by radius and shift according to voxel position.
        point[j] = (point[j]*this->m_Radius) + voxelPoint[j];
      }

      // Get the interpolated vesselness value at the sample point. If below the
      // vesselness threshold, skip (equivalent to weight of zero).
      OutputRealType vesselnessValue = m_VesselnessInterpolator->Evaluate(point);
      if (vesselnessValue < this->m_VesselnessThreshold)
        continue;
      
      // Take the absolute dot product of the eigen vector corresponding to the
      // principal direction of maximal curvature (for bright structure on dark
      // background) and the template direction.
      typename EigenVectorInterpolatorType::OutputType interpolatedVector = m_EigenVectorInterpolator->Evaluate(point);
      EigenVectorPixelType vectorValue;
      for (int j = 0; j < EigenVectorPixelType::Length; j++) {
        vectorValue[j] = interpolatedVector[j];
      }

      // The * operator is overloaded to be the dot product operator below.
      OutputRealType dot = vnl_math_abs(vectorValue * sampleDirection);

      // Weight the absolute dot product with the vesselness measure.
      sum += dot;
    }


    // Set value here.
    outputIt.Set(static_cast<OutputPixelType>(sum));

    progress.CompletedPixel();
  }

}


template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage>
typename EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>::EigenVectorSizeType
EigenVectors3DToJunctionnessImageFilter<TEigenVectorImage, TVesselnessImage, TOutputImage>
::GetLogicalRadius() {

  // Convert the radius from physical distance to logical
  // coordinate distance.
  EigenVectorSizeType logicalRadius;
  typename EigenVectorImageType::Pointer eigenVectorInput = 
    this->GetEigenVectorInput();
  EigenVectorSpacingType imageSpacing = eigenVectorInput->GetSpacing();
  for (int i = 0; i < EigenVectorImageType::ImageDimension; i++) {
    logicalRadius[i] = ceil(m_Radius / imageSpacing[i]);
  }

  return logicalRadius;
}


} // end namespace itk


#endif // _itkEigenVectors3DToJunctionnessMeasureImageFilter_cxx
