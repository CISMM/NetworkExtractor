/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenValues3DToFrangiVesselnessMeasureImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/06/23 20:13:28 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkEigenValues3DToFrangiVesselnessMeasureImageFilter_h
#define __itkEigenValues3DToFrangiVesselnessMeasureImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{

template < class TEigenValueImage, class TOutputImage >
class ITK_EXPORT EigenValues3DToFrangiVesselnessMeasureImageFilter : public
  ImageToImageFilter<TEigenValueImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef EigenValues3DToFrangiVesselnessMeasureImageFilter        Self;
  typedef ImageToImageFilter<TEigenValueImage, TOutputImage> Superclass;
  typedef SmartPointer<Self>                                 Pointer;
  typedef SmartPointer<const Self>                           ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Set/Get macros for alpha, the sensitivity to the area term (defaults to 0.5).
  Please refer to http://dx.doi.org/10.1007/BFb0056195 for more detail. */
  itkSetMacro(Alpha, double);
  itkGetMacro(Alpha, double);

  /** Set/Get macros for beta, the sensitivity to the blob term (defaults to 0.5).
  Please refer to http://dx.doi.org/10.1007/BFb0056195 for more detail*/
  itkSetMacro(Beta, double);
  itkGetMacro(Beta, double);

  /** Set/Get macros for gamma, the sensitivity to the contrast term
  (referred to as C in the paper; defaults to 1.0).
  Please refer to http://dx.doi.org/10.1007/BFb0056195 */
  itkSetMacro(Gamma, double);
  itkGetMacro(Gamma, double);

  /** Typedef for the vector type representing the eigen values. */
  typedef          TEigenValueImage                  EigenValueImageType;
  typedef typename EigenValueImageType::Pointer      EigenValueImagePointer;
  typedef typename EigenValueImageType::ConstPointer EigenValueImageConstPointer;
  typedef typename EigenValueImageType::RegionType   EigenValueImageRegionType;
  typedef typename EigenValueImageType::PixelType    EigenValueImagePixelType;

  typedef typename Superclass::OutputImageType       OutputImageType;
  typedef typename OutputImageType::Pointer          OutputImagePointer;
  typedef typename OutputImageType::ConstPointer     OutputImageConstPointer;
  typedef typename OutputImageType::RegionType       OutputImageRegionType;
  typedef typename OutputImageType::PixelType        OutputPixelType;

  /** Image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, TEigenValueImage::ImageDimension);

protected:
  EigenValues3DToFrangiVesselnessMeasureImageFilter();
  virtual ~EigenValues3DToFrangiVesselnessMeasureImageFilter() {};

  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int threadId);

private:
  EigenValues3DToFrangiVesselnessMeasureImageFilter(const Self&); // purposely not implemented
  void operator=(const Self&); // purposely not implemented

  double m_Alpha;
  double m_Beta;
  double m_Gamma;

};

} // end namespace itk

#include "itkEigenValues3DToFrangiVesselnessMeasureImageFilter.cxx"

#endif // __itkEigenValues3DToFrangiVesselnessMeasureImageFilter_h
