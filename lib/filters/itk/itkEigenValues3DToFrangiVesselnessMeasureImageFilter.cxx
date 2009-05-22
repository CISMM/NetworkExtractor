/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenValues3DToFrangiVesselnessMeasureImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/22 22:55:53 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkEigenValues3DToFrangiVesselnessMeasureImageFilter_cxx
#define _itkEigenValues3DToFrangiVesselnessMeasureImageFilter_cxx

#include "itkEigenValues3DToFrangiVesselnessMeasureImageFilter.h"

#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkProgressReporter.h>

#include <algorithm>

namespace itk {

/**
 * Constructor.
 */
template < class TEigenValueImage, class TOutputImage >
EigenValues3DToFrangiVesselnessMeasureImageFilter< TEigenValueImage, TOutputImage >
::EigenValues3DToFrangiVesselnessMeasureImageFilter()
{
  m_Alpha = 0.5;
  m_Beta  = 0.5;
  m_C     = 1.0;
}


template < class TEigenValueImage, class TOutputImage >
void
EigenValues3DToFrangiVesselnessMeasureImageFilter< TEigenValueImage, TOutputImage >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int threadId)
{
  itkDebugMacro(<< "EigenValues3DToFrangiVesselnessMeasureImageFilter generating data");

  EigenValueImageConstPointer inputPtr(
    dynamic_cast<const EigenValueImageType*>(
    (ProcessObject::GetInput(0))));

  OutputImagePointer outputPtr = this->GetOutput();

  ImageRegionConstIteratorWithIndex<EigenValueImageType> inputIt( inputPtr, outputRegionForThread );
  ImageRegionIteratorWithIndex<OutputImageType> outputIt( outputPtr, outputRegionForThread );

  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  inputIt.GoToBegin();
  outputIt.GoToBegin();

  while ( !inputIt.IsAtEnd() ) {
    EigenValueImagePixelType eigenValues = inputIt.Get();

    int minIndex = 0;
    int maxIndex = 0;
    for (int i = 1; i < 3; i++) {
      if (fabs(eigenValues[i]) < fabs(eigenValues[minIndex])) minIndex = i;
      if (fabs(eigenValues[i]) > fabs(eigenValues[maxIndex])) maxIndex = i;
    }
    int medIndex = 3 - minIndex - maxIndex;
    if (minIndex == 0 && maxIndex == 0) // all values are equal
      medIndex = 0;

    double lambda1 = eigenValues[minIndex];
    double lambda2 = eigenValues[medIndex];
    double lambda3 = eigenValues[maxIndex];

    double r_a = fabs(lambda2) / fabs(lambda3);
    double r_b = fabs(lambda1) / sqrt(fabs(lambda2*lambda3));
    double s   = sqrt(lambda1*lambda1 + lambda2*lambda2 + lambda3*lambda3);

    if (eigenValues[medIndex] > NumericTraits< OutputPixelType >::Zero 
      || eigenValues[maxIndex] > NumericTraits< OutputPixelType >::Zero) {
      outputIt.Set( NumericTraits< OutputPixelType >::Zero );
    } else {
      double v = (1.0 - vcl_exp(-((r_a*r_a)/(2*m_Alpha*m_Alpha)))) *
        vcl_exp(-((r_b*r_b)/(2*m_Beta*m_Beta))) *
        (1.0 - vcl_exp(-(s*s)/(2*m_C*m_C)));
      outputIt.Set( static_cast< OutputPixelType >(v) );
    }

    ++inputIt;
    ++outputIt;

    progress.CompletedPixel();
  }
}


} // end namespace itk

#endif // _itkEigenValues3DToFrangiVesselnessMeasureImageFilter_cxx
