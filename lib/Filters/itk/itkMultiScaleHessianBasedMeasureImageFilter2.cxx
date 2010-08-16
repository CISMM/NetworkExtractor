/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMultiScaleHessianBasedMeasureImageFilter2.cxx,v $
  Language:  C++
  Date:      $Date: 2010/01/06 04:10:30 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkMultiScaleHessianBasedMeasureImageFilter2_cxx
#define __itkMultiScaleHessianBasedMeasureImageFilter2_cxx

#include "itkMultiScaleHessianBasedMeasureImageFilter2.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "vnl/vnl_math.h"

namespace itk
{

/**
 * Constructor
 */
template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::MultiScaleHessianBasedMeasureImageFilter2()
{
  m_SigmaMinimum = 0.2;
  m_SigmaMaximum = 2.0;

  m_NumberOfSigmaSteps = 10;
  m_SigmaStepMethod = Self::LogarithmicSigmaSteps;

  m_HessianFilter = HessianFilterType::New();
  m_HessianToMeasureFilter = NULL;

  //Instantiate Update buffer
  m_UpdateBuffer = UpdateBufferType::New();

  m_GenerateScalesOutput = false;
  m_GenerateHessianOutput = false;
  m_GenerateEigenvectorOutput = false;

  typename OutputImageType::Pointer scalesImage = OutputImageType::New();
  typename HessianImageType::Pointer hessianImage = HessianImageType::New();
  typename EigenvectorImageType::Pointer eigenvectorImage = EigenvectorImageType::New();
  this->ProcessObject::SetNumberOfRequiredOutputs(4);
  this->ProcessObject::SetNthOutput(1,scalesImage.GetPointer());
  this->ProcessObject::SetNthOutput(2,hessianImage.GetPointer());
  this->ProcessObject::SetNthOutput(3,eigenvectorImage.GetPointer());
}

template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
void
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::AllocateUpdateBuffer()
{
  /* The update buffer looks just like the output and holds the best response
     in the  objectness measure */

  typename TOutputImage::Pointer output = this->GetOutput();

  m_UpdateBuffer->SetSpacing(output->GetSpacing());
  m_UpdateBuffer->SetOrigin(output->GetOrigin());
  m_UpdateBuffer->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  m_UpdateBuffer->SetRequestedRegion(output->GetRequestedRegion());
  m_UpdateBuffer->SetBufferedRegion(output->GetBufferedRegion());
  m_UpdateBuffer->Allocate();

  // Update buffer is used for > comparisons so make it really really small,
  // just to be sure. Thanks to Hauke Heibel.
  m_UpdateBuffer->FillBuffer( itk::NumericTraits< BufferValueType >::NonpositiveMin() );
}

template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
void
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::GenerateData()
{
  // Allocate the output
  this->GetOutput(0)->SetBufferedRegion(this->GetOutput(0)->GetRequestedRegion());
  this->GetOutput(0)->Allocate();

  if( m_HessianToMeasureFilter.IsNull() )
    {
    itkExceptionMacro( " HessianToMeasure filter is not set. Use SetHessianToMeasureFilter() " );
    }

  if (m_GenerateScalesOutput)
    {
    this->GetOutput(1)->SetBufferedRegion(this->GetOutput(1)->GetRequestedRegion());
    this->GetOutput(1)->Allocate();
    this->GetOutput(1)->FillBuffer(0);
    }

  if (m_GenerateHessianOutput)
    {
    this->GetOutput(2)->SetBufferedRegion(this->GetOutput(2)->GetRequestedRegion());
    this->GetOutput(2)->Allocate();
    }

  if (m_GenerateEigenvectorOutput)
    {
    EigenvectorImageType* eigenvectorOutput = dynamic_cast<EigenvectorImageType*>(this->ProcessObject::GetOutput(3));
    eigenvectorOutput->SetBufferedRegion(eigenvectorOutput->GetRequestedRegion());
    eigenvectorOutput->Allocate();
    }

  // Allocate the buffer
  AllocateUpdateBuffer();

  typename InputImageType::ConstPointer input = this->GetInput();

  this->m_HessianFilter->SetInput(input);

  this->m_HessianFilter->SetNormalizeAcrossScale(true);

  double sigma = m_SigmaMinimum;

  int scaleLevel = 1;

  while (sigma <= m_SigmaMaximum)
    {
    std::cout << "Computing measure for scale with sigma = "
              << sigma << std::endl;

    m_HessianFilter->SetSigma( sigma );

    m_HessianToMeasureFilter->SetInput ( m_HessianFilter->GetOutput() );

    m_HessianToMeasureFilter->Update();

    this->UpdateMaximumResponse(sigma);

    sigma  = this->ComputeSigmaValue( scaleLevel );

    scaleLevel++;
    }

  //Write out the best response to the output image
  ImageRegionIterator<UpdateBufferType> it(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());
  it.GoToBegin();

  ImageRegionIterator<TOutputImage> oit(this->GetOutput(0),this->GetOutput(0)->GetLargestPossibleRegion());
  oit.GoToBegin();

  while(!oit.IsAtEnd())
    {
    oit.Value() = static_cast< OutputPixelType >( it.Get() );
    ++oit;
    ++it;
    }
}

template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
void
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::UpdateMaximumResponse(double sigma)
{
  ImageRegionIterator<UpdateBufferType> oit(m_UpdateBuffer,m_UpdateBuffer->GetLargestPossibleRegion());

  ImageRegionIterator<OutputImageType>      osit; // Scales image iterator
  ImageRegionIterator<HessianImageType>     ohit; // Hessian image iterator
  ImageRegionIterator<EigenvectorImageType> oeit; // Eigenvector image iterator

  oit.GoToBegin();
  if (m_GenerateScalesOutput)
    {
    typename OutputImageType::Pointer scalesImage = static_cast<OutputImageType*>(this->ProcessObject::GetOutput(1));
    osit = ImageRegionIterator<OutputImageType>(scalesImage,scalesImage->GetLargestPossibleRegion());
    osit.GoToBegin();
    }
  if (m_GenerateHessianOutput)
    {
    typename HessianImageType::Pointer hessianImage = static_cast<HessianImageType*>(this->ProcessObject::GetOutput(2));
    ohit = ImageRegionIterator<HessianImageType>(hessianImage,hessianImage->GetLargestPossibleRegion());
    ohit.GoToBegin();
    }
  if (m_GenerateEigenvectorOutput)
    {
    typename EigenvectorImageType::Pointer eigenvectorImage = static_cast<EigenvectorImageType*>(this->ProcessObject::GetOutput(3));
    oeit = ImageRegionIterator<EigenvectorImageType>(eigenvectorImage,eigenvectorImage->GetLargestPossibleRegion());
    oeit.GoToBegin();
    }

  typedef typename HessianToMeasureFilterType::OutputImageType HessianToMeasureOutputImageType;

  ImageRegionIterator<HessianToMeasureOutputImageType> it(m_HessianToMeasureFilter->GetOutput(),
    this->m_HessianToMeasureFilter->GetOutput()->GetLargestPossibleRegion());
  ImageRegionIterator<HessianImageType> hit(m_HessianFilter->GetOutput(),
    this->m_HessianFilter->GetOutput()->GetLargestPossibleRegion());

  it.GoToBegin();
  hit.GoToBegin();

  while(!oit.IsAtEnd())
    {
    if( oit.Value() < it.Value() )
      {
      oit.Value() = it.Value();
      if (m_GenerateScalesOutput)
        {
        osit.Value() = static_cast< OutputPixelType >( sigma );
        }
      if (m_GenerateHessianOutput)
        {
        ohit.Value() = hit.Value();
        }
      if (m_GenerateEigenvectorOutput)
      {
        typename EigenvectorImageType::PixelType eigenvalues; // Borrow the eigenvector type for this
        Matrix<typename EigenvectorImageType::PixelType::ValueType, Self::ImageDimension, Self::ImageDimension> eigenvectors;
        
        // Perform the eigenanalysis on the Hessian matrix
        hit.Value().ComputeEigenAnalysis(eigenvalues, eigenvectors);

        // Find the eigenvalue with the smallest absolute value
        int smallestAbsEigenvalueIndex = 0;
        if (fabs(eigenvalues[1]) < fabs(eigenvalues[smallestAbsEigenvalueIndex]))
          smallestAbsEigenvalueIndex = 1;
        if (fabs(eigenvalues[2]) < fabs(eigenvalues[smallestAbsEigenvalueIndex]))
          smallestAbsEigenvalueIndex = 2;

        typename EigenvectorImageType::PixelType eigenvector;
        typename EigenvectorImageType::PixelType::ValueType scale;
        scale = eigenvectors[smallestAbsEigenvalueIndex][Self::ImageDimension-1] > 0 ? 1.0 : -1.0;
        for (int i = 0; i < Self::ImageDimension; i++) {
          eigenvector[i] = scale * eigenvectors[smallestAbsEigenvalueIndex][i];
        }

        oeit.Value() = eigenvector; // Make this the eigenvector
      }
      }
    ++oit;
    ++it;
    if (m_GenerateScalesOutput)
      {
      ++osit;
      }
    if (m_GenerateHessianOutput)
      {
      ++ohit;
      }
    if (m_GenerateEigenvectorOutput)
      {
      ++oeit;
      }
    if (m_GenerateHessianOutput || m_GenerateEigenvectorOutput)
      {
      ++hit;
      }
    }
}

template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
double
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::ComputeSigmaValue(int scaleLevel)
{
  double sigmaValue;

  if (m_NumberOfSigmaSteps < 2)
    {
    return m_SigmaMinimum;
    }

  switch (m_SigmaStepMethod)
    {
    case Self::EquispacedSigmaSteps:
      {
      const double stepSize = vnl_math_max(1e-10, ( m_SigmaMaximum - m_SigmaMinimum ) / (m_NumberOfSigmaSteps - 1));
      sigmaValue = m_SigmaMinimum + stepSize * scaleLevel;
      break;
      }
    case Self::LogarithmicSigmaSteps:
      {
      const double stepSize = vnl_math_max(1e-10, ( vcl_log(m_SigmaMaximum) - vcl_log(m_SigmaMinimum) ) / (m_NumberOfSigmaSteps - 1));
      sigmaValue = vcl_exp( vcl_log (m_SigmaMinimum) + stepSize * scaleLevel);
      break;
      }
    default:
      throw ExceptionObject(__FILE__, __LINE__,"Invalid SigmaStepMethod.",ITK_LOCATION);
      break;
    }

  return sigmaValue;
}

template <typename TInputImage,
          typename THessianImage, 
          typename TEigenvectorImage,
          typename TOutputImage >
void
MultiScaleHessianBasedMeasureImageFilter2
<TInputImage,THessianImage,TEigenvectorImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SigmaMinimum:  " << m_SigmaMinimum << std::endl;
  os << indent << "SigmaMaximum:  " << m_SigmaMaximum  << std::endl;
  os << indent << "NumberOfSigmaSteps:  " << m_NumberOfSigmaSteps  << std::endl;
  os << indent << "SigmaStepMethod:  " << m_SigmaStepMethod  << std::endl;
  os << indent << "HessianToMeasureFilter: " << m_HessianToMeasureFilter << std::endl;
  os << indent << "GenerateScalesOutput: " << m_GenerateScalesOutput << std::endl;
  os << indent << "GenerateHessianOutput: " << m_GenerateHessianOutput << std::endl;
  os << indent << "GenerateEigenvectorOutput: " << m_GenerateEigenvectorOutput << std::endl;
}


} // end namespace itk

#endif
