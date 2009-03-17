/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenValues3DToVesselnessMeasureImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2009/03/17 17:07:42 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkEigenValues3DToVesselnessMeasureImageFilter_cxx
#define _itkEigenValues3DToVesselnessMeasureImageFilter_cxx

#include "itkEigenValues3DToVesselnessMeasureImageFilter.h"

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
::GenerateData()
{
  itkDebugMacro(<< "EigenValues3DToVesselnessMeasureImageFilter generating data");

  EigenValueImageConstPointer inputPtr(
    dynamic_cast<const EigenValueImageType*>(
    (ProcessObject::GetInput(0))));

  OutputImagePointer outputPtr = this->GetOutput();

  outputPtr->SetBufferedRegion( inputPtr->GetBufferedRegion() );

  outputPtr->Allocate();

  EigenValueImageRegionType region = outputPtr->GetRequestedRegion();

  ImageRegionConstIteratorWithIndex<EigenValueImageType> inputIt( inputPtr, region );
  
  ImageRegionIteratorWithIndex<OutputImageType> outputIt( outputPtr, region );

  // Support progress methods/callbacks
  ProgressReporter progress(this, 0, region.GetNumberOfPixels());

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

#endif // _itkEigenValues3DToVesselnessMeasureImageFilter_cxx