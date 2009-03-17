/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenValues3DToVesselnessMeasureImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/03/17 17:07:42 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkEigenValues3DToVesselnessMeasureImageFilter_h
#define __itkEigenValues3DToVesselnessMeasureImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{

template < class TEigenValueImage, class TOutputImage >
class ITK_EXPORT EigenValues3DToVesselnessMeasureImageFilter : public
  ImageToImageFilter<TEigenValueImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef EigenValues3DToVesselnessMeasureImageFilter        Self;
  typedef ImageToImageFilter<TEigenValueImage, TOutputImage> Superclass;
  typedef SmartPointer<Self>                                 Pointer;
  typedef SmartPointer<const Self>                           ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Set/Get macros for alpha_1. Please refer to
   * http://www.spl.harvard.edu/pages/spl-pre2007/pages/papers/yoshi */
  itkSetMacro(Alpha1, double);
  itkGetMacro(Alpha1, double);
  
  /** Set/Get macros for alpha_2. Please refer to
   * http://www.spl.harvard.edu/pages/spl-pre2007/pages/papers/yoshi */
  itkSetMacro(Alpha2, double);
  itkGetMacro(Alpha2, double);

  /** Typedef for the vector type representing the eigen values. */
  typedef          TEigenValueImage                  EigenValueImageType;
  typedef typename EigenValueImageType::Pointer      EigenValueImagePointer;
  typedef typename EigenValueImageType::ConstPointer EigenValueImageConstPointer;
  typedef typename EigenValueImageType::RegionType   EigenValueImageRegionType;
  typedef typename EigenValueImageType::PixelType    EigenValueImagePixelType;

  typedef typename Superclass::InputImageType        InputImageType;
  typedef typename Superclass::OutputImageType       OutputImageType;
  typedef typename OutputImageType::Pointer          OutputImagePointer;
  typedef typename OutputImageType::ConstPointer     OutputImageConstPointer;
  typedef typename OutputImageType::RegionType       OutputImageRegionType;
  typedef typename OutputImageType::PixelType        OutputPixelType;

  /** Image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, TEigenValueImage::ImageDimension);

protected:
  EigenValues3DToVesselnessMeasureImageFilter();
  virtual ~EigenValues3DToVesselnessMeasureImageFilter() {};

  void GenerateData( void );

private:
  EigenValues3DToVesselnessMeasureImageFilter(const Self&); // purposely not implemented
  void operator=(const Self&); // purposely not implemented

  double m_Alpha1;
  double m_Alpha2;

};

} // end namespace itk

#include "itkEigenValues3DToVesselnessMeasureImageFilter.cxx"

#endif // __itkEigenValues3DToVesselnessMeasureImageFilter_h