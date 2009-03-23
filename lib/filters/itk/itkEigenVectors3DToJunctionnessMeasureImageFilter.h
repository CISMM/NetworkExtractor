/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenVectors3DToJunctionnessMeasureImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/03/23 00:00:50 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkEigenVectors3DToJunctionnessMeasureImageFilter_h
#define __itkEigenVectors3DToJunctionnessMeasureImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk {


/** \class EigenVectors3DToJunctionnessMeasureImageFilter
 * \brief Implements a junctionness matched filter to identify junction points
 * in images of fibrous networks.
 *
 * The class takes a tensor image of eigen vectors computed from a Hessian matrix 
 * image and an scalar image giving a weighting on vesselness/fiberness. The
 * matched filter computes the sum of absolute dot products of the eigen vectors and
 * sphere normal in a local neighborhood. Each dot product is weighted by the
 * vesselness measure to reduce spurious junction identification.
 *
 * \ingroup Multithreaded
 */

template <class TEigenVectorImage, class TVesselnessImage, class TOutputImage=TVesselnessImage>
class EigenVectors3DToJunctionnessImageFilter :
  public ImageToImageFilter<TEigenVectorImage, TOutputImage> {

public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(EigenVectorImageDimension, unsigned int,
    TEigenVectorImage::ImageDimension);
  itkStaticConstMacro(VesselnessImageDimension, unsigned int,
    TVesselnessImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TEigenVectorImage EigenVectorImageType;
  typedef TVesselnessImage  VesselnessImageType;
  typedef TOutputImage      OutputImageType;

  /** Standard class typedefs. */
  typedef EigenVectors3DToJunctionnessImageFilter Self;
  typedef ImageToImageFilter< EigenVectorImageType, OutputImageType > Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(EigenVectors3DToJunctionnessImageFilter, ImageToImageFilter);

  /** Image typedef support. */
  typedef typename EigenVectorImageType::PixelType  EigenVectorPixelType;
  typedef typename VesselnessImageType::PixelType   VesselnessPixelType;
  typedef typename OutputImageType::PixelType       OutputPixelType;

  typedef typename EigenVectorImageType::RegionType EigenVectorImageRegionType;
  typedef typename VesselnessImageType::RegionType  VesselnessImageRegionType;
  typedef typename OutputImageType::RegionType      OutputImageRegionType;

  typedef typename EigenVectorImageType::SizeType   EigenVectorSizeType;
  typedef typename VesselnessImageType::SizeType    VesselnessSizeType;

  /** Set the radius of the neighborhood used to compute the junctionness measure. */
  itkSetMacro(Radius, EigenVectorSizeType);

  /** Get the radius of the neighborhood used to compute the junctionness measure. */
  itkGetConstReferenceMacro(Radius, EigenVectorSizeType);

  /** Set/get the eigen vector input image. */
  typename void SetEigenVectorInput(typename EigenVectorImageType::Pointer input);
  typename EigenVectorImageType::Pointer GetEigenVectorInput();

  /** Set/get the vesselness input image. */
  typename void SetVesselnessInput(typename VesselnessImageType::Pointer input);
  typename VesselnessImageType::Pointer GetVesselnessInput();

  /** EigenVectors3DToJunctionnessImageFilter needs a larger input requested
   * region than the output requested region. As such, EigenVectors3DToJunctionnessImageFilter
   * needs to provide an implementation for GenerateInputRequestedRegion() in
   * order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);

protected:
  EigenVectors3DToJunctionnessImageFilter();
  virtual ~EigenVectors3DToJunctionnessImageFilter() {}
  
  /** This filter can be implemented as a multithreaded filter. */
  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
    int threadId);

private:
  EigenVectors3DToJunctionnessImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  EigenVectorSizeType m_Radius;
};

} // end namespace itk

#include "itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx"


#endif // __itkEigenVectors3DToJunctionnessMeasureImageFilter_h