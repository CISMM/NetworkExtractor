/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHessian3DEigenAnalysisImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/03/19 15:44:09 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkHessian3DEigenAnalysisImageFilter_h
#define __itkHessian3DEigenAnalysisImageFilter_h

#include "itkImageToImageFilter.h"


namespace itk {

/** \class Hessian3DEigenAnalysisImageFilter
 * \brief Compute pixel-wise the eigen values and eigen vectors of a 3D Hessian matrix.
 *
 * This filter expects a tensor image as produced by HessianRecursiveGaussianImageFilter.
 *
 * The eigen values are stored in one output image and the eigen vectors are stored
 * was a second output image.
 *
 */

template <class TInputImage, class TEigenValueImage, class TEigenVectorImage>
class ITK_EXPORT Hessian3DEigenAnalysisImageFilter :
  public ImageToImageFilter<TInputImage, TEigenValueImage>
{
public:
  /** Standard class typedefs. */
  typedef Hessian3DEigenAnalysisImageFilter                Self;
  typedef ImageToImageFilter<TInputImage,TEigenValueImage> Superclass;
  typedef SmartPointer<Self>                               Pointer;
  typedef SmartPointer<const Self>                         ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Typedef for the vector type representing the eigen values. */
  typedef          TEigenValueImage                 EigenValueImageType;
  typedef typename EigenValueImageType::Pointer     EigenValueImagePointer;
  typedef typename EigenValueImageType::RegionType  EigenValueImageRegionType;
  typedef typename EigenValueImageType::PixelType   EigenValueImagePixelType;

  /** Typedef for the vector type representing the eigen vectors. */
  typedef          TEigenVectorImage                EigenVectorImageType;
  typedef typename EigenVectorImageType::Pointer    EigenVectorImagePointer;
  typedef typename EigenVectorImageType::RegionType EigenVectorImageRegionType;
  typedef typename EigenVectorImageType::PixelType  EigenVectorImagePixelType;

  /** Superclass typedefs. */
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  /** Image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Get the output image with the eigen values. */
  EigenValueImageType* GetEigenValues( void );

  /** Get the output image with the eigen vectors stored in 3x3 matrix form. */
  EigenVectorImageType* GetEigenVectors( void );

  /** Create the output. */
  DataObject::Pointer MakeOutput(unsigned int idx);

protected:
  Hessian3DEigenAnalysisImageFilter();
  virtual ~Hessian3DEigenAnalysisImageFilter() {};

  virtual void AllocateOutputs();

  void ThreadedGenerateData(const EigenValueImageRegionType& outputRegionForThread, int threadId);

private:
  Hessian3DEigenAnalysisImageFilter(const Self&); // purposely not implemented
  void operator=(const Self&); // purposely not implemented


};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessian3DEigenAnalysisImageFilter.cxx"
#endif

#endif // __itkHessian3DEigenAnalysisImageFilter_h