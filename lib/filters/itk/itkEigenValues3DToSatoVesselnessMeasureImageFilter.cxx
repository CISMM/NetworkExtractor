/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenValues3DToSatoVesselnessMeasureImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/05/21 12:49:12 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkEigenValues3DToSatoVesselnessMeasureImageFilter_cxx
#define _itkEigenValues3DToSatoVesselnessMeasureImageFilter_cxx

#include "itkEigenValues3DToSatoVesselnessMeasureImageFilter.h"

namespace itk {

/**
 * Constructor.
 */
template < class TEigenValueImage, class TOutputImage >
EigenValues3DToVesselnessMeasureImageFilter< TEigenValueImage, TOutputImage >
::EigenValues3DToVesselnessMeasureImageFilter()
{
  m_Alpha1 = 0.5;
  m_Alpha2 = 2.0;
}


template < class TEigenValueImage, class TOutputImage >
void
EigenValues3DToVesselnessMeasureImageFilter< TEigenValueImage, TOutputImage >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int threadId)
{
  itkDebugMacro(<< "EigenValues3DToVesselnessMeasureImageFilter generating data");

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
    EigenValueImagePixelType eigenValue = inputIt.Get();

    // normalizeValue <= 0 for bright line structures
    double normalizeValue = vnl_math_min( -1.0 * eigenValue[1], -1.0 * eigenValue[0]);

    // Similarity measure to a line structure
    if ( normalizeValue > 0 ) {
      double lineMeasure;
      if (eigenValue[2] <= 0) {
        lineMeasure = 
          vcl_exp(-0.5 * vnl_math_sqr( eigenValue[2] / (m_Alpha1 * normalizeValue)));
      } else {
        lineMeasure =
          vcl_exp(-0.5 * vnl_math_sqr( eigenValue[2] / (m_Alpha2 * normalizeValue)));
      }

      lineMeasure *= normalizeValue;
      outputIt.Set( static_cast< OutputPixelType >(lineMeasure) );
    } else {
      outputIt.Set( NumericTraits< OutputPixelType >::Zero );
    }

    ++inputIt;
    ++outputIt;

    progress.CompletedPixel();
  }
}


} // end namespace itk

#endif // _itkEigenValues3DToSatoVesselnessMeasureImageFilter_cxx