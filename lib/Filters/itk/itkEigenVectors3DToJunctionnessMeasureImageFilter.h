/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEigenVectors3DToJunctionnessMeasureImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/05/22 22:55:53 $
  Version:   $Revision: 1.4 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkEigenVectors3DToJunctionnessMeasureImageFilter_h
#define __itkEigenVectors3DToJunctionnessMeasureImageFilter_h

//#include "itkBSplineInterpolateImageFunction.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageToImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMesh.h"
#include "itkRegularSphereMeshSource.h"
#include "itkVectorLinearInterpolateImageFunction.h"

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
  typedef typename EigenVectorImageType::Pointer      EigenVectorImagePointer;
  typedef typename VesselnessImageType::Pointer       VesselnessImagePointer;
  typedef typename OutputImageType::Pointer           OutputImagePointer;

  typedef typename EigenVectorImageType::ConstPointer EigenVectorImageConstPointer;
  typedef typename VesselnessImageType::ConstPointer  VesselnessImageConstPointer;
  
  typedef typename EigenVectorImageType::PixelType    EigenVectorPixelType;
  typedef typename VesselnessImageType::PixelType     VesselnessPixelType;
  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename NumericTraits<OutputPixelType>::RealType OutputRealType;

  typedef typename EigenVectorImageType::RegionType   EigenVectorImageRegionType;
  typedef typename VesselnessImageType::RegionType    VesselnessImageRegionType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  typedef typename EigenVectorImageType::SizeType     EigenVectorSizeType;
  typedef typename VesselnessImageType::SizeType      VesselnessSizeType;

  typedef typename EigenVectorImageType::SpacingType  EigenVectorSpacingType;
  typedef typename VesselnessImageType::SpacingType   VesselnessSpacingType;

  typedef ImageRegionConstIteratorWithIndex<EigenVectorImageType> EigenVectorConstIteratorWithIndex;
  typedef ImageRegionConstIteratorWithIndex<VesselnessImageType>  VesselnessConstIteratorWithIndex;
  typedef ImageRegionIteratorWithIndex<OutputImageType>           OutputImageIteratorWithIndex;

  typedef Mesh<double> MeshType;
  typedef RegularSphereMeshSource<MeshType>                 SphereSourceType;
  typedef typename SphereSourceType::Pointer                SphereSourcePointer;
  typedef typename SphereSourceType::PointsContainer        SphereSourcePointsContainer;
  typedef typename SphereSourceType::PointsContainerPointer SphereSourcePointsContainerPointer;

  typedef VectorLinearInterpolateImageFunction<EigenVectorImageType> EigenVectorInterpolatorType;
  typedef typename EigenVectorInterpolatorType::Pointer              EigenVectorInterpolatorPointer;
  typedef LinearInterpolateImageFunction<VesselnessImageType, float> VesselnessInterpolatorType;
  typedef typename VesselnessInterpolatorType::Pointer               VesselnessInterpolatorPointer;

  /** Set/get the radius of the neighborhood used to compute the junctionness measure. */
  itkSetMacro(Radius, double);
  itkGetConstReferenceMacro(Radius, double);

  /** Set/get the vesselness measure above which the associated eigenvectors are 
   considered by the junctionness probe. */
  itkSetMacro(VesselnessThreshold, double);
  itkGetConstReferenceMacro(VesselnessThreshold, double);

  /** Set/get the eigenvector input image. */
  void SetEigenVectorInput(typename EigenVectorImageType::Pointer input);
  typename EigenVectorImageType::Pointer GetEigenVectorInput();

  /** Set/get the vesselness input image. */
  void SetVesselnessInput(typename VesselnessImageType::Pointer input);
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

  /** Convert physical spacing to logical spacing in image coordinates. */
  EigenVectorSizeType GetLogicalRadius();

private:
  EigenVectors3DToJunctionnessImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  // Radius of junction probe filter
  double m_Radius;

  // Vesselness measure above which the associated eigen vectors are 
  // considered by the junctionness probe.
  double m_VesselnessThreshold;

  SphereSourcePointer m_SphereSampleSource;

  VesselnessInterpolatorPointer m_VesselnessInterpolator;

  EigenVectorInterpolatorPointer m_EigenVectorInterpolator;

};

} // end namespace itk

#include "itkEigenVectors3DToJunctionnessMeasureImageFilter.cxx"


#endif // __itkEigenVectors3DToJunctionnessMeasureImageFilter_h
