#ifndef _DATA_MODEL_CXX_
#define _DATA_MODEL_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <cstdlib>

#include "DataModel.h"
#include "Types.h"

#include <itkAccumulateImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkMetaDataObject.h>
#include <itkMultiThreader.h>
#include <itkPoint.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

// Constants
const DataModel::FilterType 
DataModel::NO_FILTER_STRING("No filter");

const DataModel::FilterType
DataModel::FRANGI_FIBERNESS_FILTER_STRING("Frangi Fiberness Filter");

const DataModel::FilterType
DataModel::MULTISCALE_FIBERNESS_FILTER_STRING("Multiscale Fiberness");

const DataModel::FilterType
DataModel::MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING("Multiscale Fiberness Threshold");

const DataModel::FilterType
DataModel::MULTISCALE_SKELETONIZATION_FILTER_STRING("Skeletonization");

const DataModel::FilterType
DataModel::JUNCTIONNESS_FILTER_STRING("Junctionness");

const DataModel::FilterType
DataModel::JUNCTIONNESS_LOCAL_MAX_FILTER_STRING("Junctionness Local Maxima");


DataModel
::DataModel() {
  m_FiberDiameter = 1.0f;
  m_FilteredImageScale = 1.0;
  m_RefDirectionAzimuth = 0.0;
  m_RefDirectionInclination = 0.0;
  m_ZSquishFactor = 1.0;

  m_ImageData = NULL;

  m_ResampleFilter = ResampleFilterType::New();
  m_MinMaxFilter = MinMaxType::New();

  // Resize the z-spacing filter.
  m_SquishZSpacingFilter = ChangeInfoFilterType::New();
  m_SquishZSpacingFilter->ChangeDirectionOff();
  m_SquishZSpacingFilter->ChangeOriginOff();
  m_SquishZSpacingFilter->ChangeRegionOff();
  m_SquishZSpacingFilter->ChangeSpacingOn();
  m_SquishZSpacingFilter->CenterImageOff();
  m_SquishZSpacingFilter->SetInput(m_ResampleFilter->GetOutput());

  // Fiberness filters.
  m_HessianFilter = HessianFilterType::New();
  m_HessianFilter->SetInput(m_SquishZSpacingFilter->GetOutput());

  m_EigenAnalysisFilter = HessianEigenAnalysisFilterType::New();
  m_EigenAnalysisFilter->SetInput(m_HessianFilter->GetOutput());
  
  m_FibernessFilter = FibernessFilterType::New();
  m_FibernessFilter->SetInput(m_EigenAnalysisFilter->GetEigenValues());

  m_HessianToVesselnessFilter = HessianToObjectnessFilterType::New();
  m_HessianToVesselnessFilter->SetObjectDimension(1);
  m_HessianToVesselnessFilter->ScaleObjectnessMeasureOff();
  m_HessianToVesselnessFilter->SetBrightObject(true);

  m_MultiscaleFibernessFilter = MultiScaleHessianMeasureImageType::New();
  m_MultiscaleFibernessFilter->GenerateHessianOutputOff();
  m_MultiscaleFibernessFilter->GenerateScalesOutputOff();
  m_MultiscaleFibernessFilter->GenerateEigenvectorOutputOn();
  m_MultiscaleFibernessFilter->SetSigmaMinimum(0.5);
  m_MultiscaleFibernessFilter->SetSigmaMaximum(1.5);
  m_MultiscaleFibernessFilter->SetSigmaStepMethodToEquispaced();
  m_MultiscaleFibernessFilter->SetNumberOfSigmaSteps(11);
  m_MultiscaleFibernessFilter->SetHessianToMeasureFilter(m_HessianToVesselnessFilter);
  m_MultiscaleFibernessFilter->SetInput(m_SquishZSpacingFilter->GetOutput());

  m_MultiscaleFibernessThresholdFilter = ThresholdFilterType::New();
  m_MultiscaleFibernessThresholdFilter->SetInsideValue(0.0);
  m_MultiscaleFibernessThresholdFilter->SetOutsideValue(1.0);
  m_MultiscaleFibernessThresholdFilter->SetLowerThreshold(0.0);
  m_MultiscaleFibernessThresholdFilter->SetUpperThreshold(0.0);
  m_MultiscaleFibernessThresholdFilter->SetInput(m_MultiscaleFibernessFilter->GetOutput());

  m_SkeletonizationFilter = SkeletonizationFilterType::New();
  m_SkeletonizationFilter->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());

  m_FibernessConnectedComponentsFilter = ConnectedComponentFilterType::New();
  m_FibernessConnectedComponentsFilter->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());
  m_FibernessConnectedComponentsFilter->FullyConnectedOn();

  m_MinMaxConnectedComponentsFilter = MinMaxConnectedComponentFilterType::New();

  m_JunctionnessFilter = JunctionnessFilterType::New();
  m_JunctionnessFilter->SetEigenVectorInput(m_EigenAnalysisFilter->GetEigenVectors());
  m_JunctionnessFilter->SetVesselnessInput(m_FibernessFilter->GetOutput());

  m_JunctionnessLocalMaxFilter = JunctionnessLocalMaxFilterType::New();
  m_JunctionnessLocalMaxFilter->SetInput(m_JunctionnessFilter->GetOutput());
  m_JunctionnessLocalMaxFilter->FullyConnectedOn();

  m_InputImageITKToVTKFilter = new ITKImageToVTKImage<TImage>();
  m_FilteredImageITKToVTKFilter = new ITKImageToVTKImage<TImage>();
  m_VectorImageITKToVTKFilter = new ITKImageToVTKImage<EigenVectorImageType>();
  progressCallback = NULL;

    // Hook up progress reporter for the filters.
  itk::MemberCommand< DataModel >::Pointer m_HessianFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_EigenAnalysisFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_FibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_MultiscaleFibernessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer fibernessThresholdFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_SkeletonizationFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_FibernessConnectedComponentsFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_JunctionnessFilterProgressCommand 
    = itk::MemberCommand< DataModel >::New();

  itk::MemberCommand< DataModel >::Pointer m_JunctionnessLocalMaxFilterProgressCommand
    = itk::MemberCommand< DataModel >::New();

  // Set the callback function for each of the progress reporters.
  m_HessianFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_EigenAnalysisFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_FibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_MultiscaleFibernessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  fibernessThresholdFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_SkeletonizationFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_FibernessConnectedComponentsFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_JunctionnessFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);
  m_JunctionnessLocalMaxFilterProgressCommand->SetCallbackFunction(this, &DataModel::UpdateProgress);

  // Attach a MetaDataObject with the name of the filter to each rilter.
  itk::EncapsulateMetaData<std::string >(m_HessianFilter->GetMetaDataDictionary(),
    "filterName", std::string("Hessian Filter"));
  itk::EncapsulateMetaData<std::string >(m_EigenAnalysisFilter->GetMetaDataDictionary(),
    "filterName", std::string("Eigen Analysis Filter"));
  itk::EncapsulateMetaData<std::string >(m_FibernessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Filter"));
  itk::EncapsulateMetaData<std::string >(m_MultiscaleFibernessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Multiscale Fiberness Filter"));
  itk::EncapsulateMetaData<std::string >(m_MultiscaleFibernessThresholdFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Threshold Filter"));
  itk::EncapsulateMetaData<std::string >(m_SkeletonizationFilter->GetMetaDataDictionary(),
    "filterName", std::string("Skeletonization Filter"));
  itk::EncapsulateMetaData<std::string >(m_FibernessConnectedComponentsFilter->GetMetaDataDictionary(),
    "filterName", std::string("Fiberness Connected Components Filter"));
  itk::EncapsulateMetaData<std::string >(m_JunctionnessFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Filter"));
  itk::EncapsulateMetaData<std::string>(m_JunctionnessLocalMaxFilter->GetMetaDataDictionary(),
    "filterName", std::string("Junctionness Local Max Filter"));

  m_HessianFilter->AddObserver(itk::ProgressEvent(), m_HessianFilterProgressCommand);
  m_EigenAnalysisFilter->AddObserver(itk::ProgressEvent(), m_EigenAnalysisFilterProgressCommand);
  m_FibernessFilter->AddObserver(itk::ProgressEvent(), m_FibernessFilterProgressCommand);
  m_MultiscaleFibernessFilter->AddObserver(itk::ProgressEvent(), m_MultiscaleFibernessFilterProgressCommand);
  m_MultiscaleFibernessThresholdFilter->AddObserver(itk::ProgressEvent(), fibernessThresholdFilterProgressCommand);
  m_SkeletonizationFilter->AddObserver(itk::ProgressEvent(), m_SkeletonizationFilterProgressCommand);
  m_FibernessConnectedComponentsFilter->AddObserver(itk::ProgressEvent(), m_FibernessConnectedComponentsFilterProgressCommand);
  m_JunctionnessFilter->AddObserver(itk::ProgressEvent(), m_JunctionnessFilterProgressCommand);
  m_JunctionnessLocalMaxFilter->AddObserver(itk::ProgressEvent(), m_JunctionnessLocalMaxFilterProgressCommand);

  // ITK will detect the number of cores on the system and set it by default.
  // Here we can override that setting if the proper environment variable is
  // set.
  char *var = getenv("NETWORK_EXTRACTOR_THREADS");
  if (var) {
    int numberOfThreads = atoi(var);
    if (numberOfThreads > 0)
      SetNumberOfThreads(numberOfThreads);
  }
}


DataModel
::~DataModel() {
  delete m_InputImageITKToVTKFilter;
  delete m_FilteredImageITKToVTKFilter;
  delete m_VectorImageITKToVTKFilter;
}


void
DataModel
::LoadImageFile(std::string fileName) {
  m_ImageFileName = fileName;
  ScalarFileReaderType::Pointer reader = ScalarFileReaderType::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();

  SetImageData(reader->GetOutput());

  // Connect this image data to the various pipelines.
  m_MinMaxFilter->SetImage(GetImageData());
  m_MinMaxFilter->Compute();

  m_InputImageITKToVTKFilter->Modified();
  m_InputImageITKToVTKFilter->Update();
  m_FilteredImageITKToVTKFilter->Modified();
  m_FilteredImageITKToVTKFilter->Update();
}


void
DataModel
::SaveFilteredImageFile(std::string fileName, std::string filterName, float scale) {
  // Check for TIFF output and perform casting as needed.
  size_t tiffLocation = fileName.rfind(".tif");
  if (tiffLocation == fileName.size()-4) {
    ScaleFilterType::Pointer scaler = ScaleFilterType::New();
    scaler->SetShift(0.0f);
    scaler->SetScale(scale);

    if (filterName == NO_FILTER_STRING) {
      scaler->SetInput(m_ImageData);
    } else if (filterName == FRANGI_FIBERNESS_FILTER_STRING) {
      scaler->SetInput(m_FibernessFilter->GetOutput());
    } else if (filterName == MULTISCALE_FIBERNESS_FILTER_STRING) {
      scaler->SetInput(m_MultiscaleFibernessFilter->GetOutput());
    } else if (filterName == MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING) {
      scaler->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());
    } else if (filterName == MULTISCALE_SKELETONIZATION_FILTER_STRING) {
      scaler->SetInput(m_SkeletonizationFilter->GetOutput());
    } else if (filterName == JUNCTIONNESS_FILTER_STRING) {
      scaler->SetInput(m_JunctionnessFilter->GetOutput());
    } else if (filterName == JUNCTIONNESS_LOCAL_MAX_FILTER_STRING) {
      scaler->SetInput(m_JunctionnessLocalMaxFilter->GetOutput());
    } else {
      return;
    }

    UShort3DCastType::Pointer caster = UShort3DCastType::New();
    caster->SetInput(scaler->GetOutput());

    TIFFWriterType::Pointer writer = TIFFWriterType::New();
    writer->SetInput(caster->GetOutput());
    writer->SetFileName(fileName.c_str());
    writer->Update();

  } else { // Nothing special to do for VTK output files.

    ScalarFileWriterType::Pointer writer = ScalarFileWriterType::New();
    if (filterName == NO_FILTER_STRING)
      writer->SetInput(m_ImageData);
    else if (filterName == FRANGI_FIBERNESS_FILTER_STRING)
      writer->SetInput(m_FibernessFilter->GetOutput());
    else if (filterName == MULTISCALE_FIBERNESS_FILTER_STRING)
      writer->SetInput(m_MultiscaleFibernessFilter->GetOutput());
    else if (filterName == MULTISCALE_FIBERNESS_THRESHOLD_FILTER_STRING)
      writer->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());
    else if (filterName == MULTISCALE_SKELETONIZATION_FILTER_STRING)
      writer->SetInput(m_SkeletonizationFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_FILTER_STRING)
      writer->SetInput(m_JunctionnessFilter->GetOutput());
    else if (filterName == JUNCTIONNESS_LOCAL_MAX_FILTER_STRING)
      writer->SetInput(m_JunctionnessLocalMaxFilter->GetOutput());
    else
      return;

    writer->SetFileName(fileName.c_str());
    writer->Update();
  }
}


std::string
DataModel
::GetImageFileName() {
  return m_ImageFileName;
}


void
DataModel
::SetNumberOfThreads(int threads) {
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(threads);
}


int
DataModel
::GetNumberOfThreads() {
  return itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
}


void
DataModel
::SaveFiberOrientationImageFile(std::string fileName) {
  VectorFileWriterType::Pointer writer = VectorFileWriterType::New();
  writer->SetInput(m_MultiscaleFibernessFilter->GetEigenvectorOutput());
  writer->SetFileName(fileName.c_str());
  writer->Update();
  writer->Write();
}


void
DataModel
::SaveFiberOrientationDataFile(std::string fileName) {
  InputImageType::Pointer skeletonImage = m_SkeletonizationFilter->GetOutput();
  itk::ImageRegionIteratorWithIndex<InputImageType> sit(skeletonImage,skeletonImage->GetLargestPossibleRegion());

  const EigenVectorImageType* eigenvectorImage = m_MultiscaleFibernessFilter->GetEigenvectorOutput();
  itk::ImageRegionConstIteratorWithIndex<EigenVectorImageType> eit(eigenvectorImage,eigenvectorImage->GetLargestPossibleRegion());

  sit.GoToBegin();
  eit.GoToBegin();

  // Open file.
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "xPosition,yPosition,zPosition,xDirection,yDirection,zDirection\n");

  while(!sit.IsAtEnd()) {
    if (sit.Value() > 0.0) {
      itk::Index<3> index = eit.GetIndex();
      itk::Point<float, 3> point;
      m_ImageData->TransformIndexToPhysicalPoint(index, point);
      EigenVectorType eigenvector = eit.Value();
      double ex = eigenvector[0];
      double ey = eigenvector[1];
      double ez = eigenvector[2] / m_ZSquishFactor;

      // Renormalize
      double len = sqrt(ex*ex + ey*ey + ez*ez);
      ex /= len;   ey /= len;   ez /= len;

      fprintf(fp, "%f,%f,%f,%f,%f,%f\n", point[0], point[1], point[2], ex, ey, ez);

    }

    ++sit;
    ++eit;
  }

  fclose(fp);
}


void
DataModel
::SaveFiberAngleHistogram(std::string fileName, double azimuth,
			  double inclination, unsigned int bins) {

  if (bins < 2)
    bins = 2;

  // Allocate and initialize the histogram
  unsigned int *hist = new unsigned int[bins];
  for (unsigned int i = 0; i < bins; i++) hist[i] = 0;

  // Calculate the angle spread of each histogram bin
  float angleIncrement = 90.0f / static_cast<float>(bins);

  // Convert from azimuth-inclination to 3D Euclidean vector
  double phi   = M_PI * m_RefDirectionAzimuth / 180.0;
  double theta = M_PI * (90-m_RefDirectionInclination) / 180.0;
  double rx = cos(phi) * sin(theta);
  double ry = sin(phi) * sin(theta);
  double rz = cos(theta);

  InputImageType::Pointer skeletonImage = m_SkeletonizationFilter->GetOutput();
  itk::ImageRegionIteratorWithIndex<InputImageType> sit(skeletonImage,skeletonImage->GetLargestPossibleRegion());

  const EigenVectorImageType* eigenvectorImage = m_MultiscaleFibernessFilter->GetEigenvectorOutput();
  itk::ImageRegionConstIteratorWithIndex<EigenVectorImageType> eit(eigenvectorImage,eigenvectorImage->GetLargestPossibleRegion());

  sit.GoToBegin();
  eit.GoToBegin();

  while(!eit.IsAtEnd()) {
    if (sit.Value() > 0.0) {
      EigenVectorType e = eit.Value();

      // We need to rescale the z-component of the vector by the inverse
      // of the z-squish factor.
      double ex = e[0];
      double ey = e[1];
      double ez = e[2] / m_ZSquishFactor;
      
      // Renormalize
      double len = sqrt(ex*ex + ey*ey + ez*ez);
      ex /= len;   ey /= len;   ez /= len;
      
      // Compute the angle in degrees.
      double angle = 180.0*acos(fabs(ex*rx + ey*ry + ez*rz)) / M_PI;

      // Update the histogram, which goes from 0 to 90 degrees.
      unsigned int theBin = static_cast<unsigned int>(angle / angleIncrement);
      if (theBin < 0)     theBin = 0;
      if (theBin >= bins) theBin = bins - 1;
      hist[theBin]++;
    }

    ++sit;
    ++eit;
  }

  // Open file and write out the histogram.
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "Angle (deg.),Count,Normalized Count,Expected Probability,Over-representation Ratio\n");

  // First, sum up total counts for the histogram normalization
  double countSum = 0.0;
  for (unsigned int i = 0; i < bins; i++) {
    countSum += static_cast<double>(hist[i]);
  }

  for (unsigned int i = 0; i < bins; i++) {
    double angleA = angleIncrement * static_cast<float>(i);
    double angleB = angleIncrement * static_cast<float>(i+1);
    double normalizedHist = static_cast<double>(hist[i]) / countSum;
    double expectedHist   = cos(M_PI*angleA/180.0) - cos(M_PI*angleB/180.0);
    double ratio = normalizedHist / expectedHist;
    fprintf(fp, "%.2f, %d, %f, %f, %f\n", angleA, hist[i], normalizedHist,
	    expectedHist, ratio);
  }
  
  fclose(fp);
}


void
DataModel
::SetFiberDiameter(double diameter) {
  if (diameter != m_FiberDiameter) {
    m_FiberDiameter = diameter;

    // Set sigma on the Hessian filter.
    m_HessianFilter->SetSigma(0.5*diameter);
  }
}


void
DataModel
::SetMultiscaleFibernessAlphaCoefficient(double alpha) {
  if (alpha != m_FibernessFilter->GetAlpha())
    m_FibernessFilter->SetAlpha(alpha);
  if (alpha != m_HessianToVesselnessFilter->GetAlpha()) {
    m_HessianToVesselnessFilter->SetAlpha(alpha);
    m_MultiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessAlphaCoefficient() {
  return m_FibernessFilter->GetAlpha();
}


void
DataModel
::SetMultiscaleFibernessBetaCoefficient(double beta) {
  if (beta != m_FibernessFilter->GetBeta())
    m_FibernessFilter->SetBeta(beta);
  if (beta != m_HessianToVesselnessFilter->GetBeta()) {
    m_HessianToVesselnessFilter->SetBeta(beta);
    m_MultiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessBetaCoefficient() {
  return m_FibernessFilter->GetBeta();
}


void
DataModel
::SetMultiscaleFibernessGammaCoefficient(double gamma) {
  if (gamma != m_FibernessFilter->GetGamma())
    m_FibernessFilter->SetGamma(gamma);
  if (gamma != m_HessianToVesselnessFilter->GetGamma()) {
    m_HessianToVesselnessFilter->SetGamma(gamma);
    m_MultiscaleFibernessFilter->Modified();
  }
}


double
DataModel
::GetMultiscaleFibernessGammaCoefficient() {
  return m_FibernessFilter->GetGamma();
}


void
DataModel
::SetMultiscaleFibernessMinimumScale(double minimum) {
  if (minimum != m_MultiscaleFibernessFilter->GetSigmaMinimum()) {
    m_MultiscaleFibernessFilter->SetSigmaMinimum(minimum);
  }
}


double
DataModel
::GetMultiscaleFibernessMinimumScale() {
  return m_MultiscaleFibernessFilter->GetSigmaMinimum();
}


void
DataModel
::SetMultiscaleFibernessMaximumScale(double maximum) {
  if (maximum != m_MultiscaleFibernessFilter->GetSigmaMaximum()) {
    m_MultiscaleFibernessFilter->SetSigmaMaximum(maximum);
  }
}


double
DataModel
::GetMultiscaleFibernessMaximumScale() {
  return m_MultiscaleFibernessFilter->GetSigmaMaximum();
}


void
DataModel
::SetMultiscaleFibernessNumberOfScales(int numberOfScales) {
  m_MultiscaleFibernessFilter->SetNumberOfSigmaSteps(numberOfScales);
}


int
DataModel
::GetMultiscaleFibernessNumberOfScales() {
  return m_MultiscaleFibernessFilter->GetNumberOfSigmaSteps();
}


void
DataModel
::SetMultiscaleFibernessThreshold(double threshold) {
  m_MultiscaleFibernessThresholdFilter->SetLowerThreshold(0.0);
  m_MultiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
}


double
DataModel
::GetMultiscaleFibernessThreshold() {
  return m_MultiscaleFibernessThresholdFilter->GetUpperThreshold();
}


double
DataModel
::GetFiberDiameter() {
  return m_FiberDiameter;
}


void
DataModel
::SetJunctionProbeDiameter(double diameter) {
  // Set probe radius on the junctionness filter.
  if (0.5*diameter != m_JunctionnessFilter->GetRadius()) {
    m_JunctionnessFilter->SetRadius(0.5*diameter);
  }
}


double
DataModel
::GetJunctionProbeDiameter() {
  return 2.0*m_JunctionnessFilter->GetRadius();
}


void
DataModel
::SetJunctionFibernessThreshold(double threshold) {
  if (threshold != m_JunctionnessFilter->GetVesselnessThreshold()) {
    m_JunctionnessFilter->SetVesselnessThreshold(threshold);
  }
}


double
DataModel
::GetJunctionFibernessThreshold() {
  return m_JunctionnessFilter->GetVesselnessThreshold();
}


// Deprecated method
void
DataModel
::SetJunctionnessLocalMaxHeight(double height) {
  //if (height != m_JunctionnessLocalMaxFilter->GetHeight()) {
  //  m_JunctionnessLocalMaxFilter->SetHeight(height);
  //}
}


// Deprecated method
double
DataModel
::GetJunctionnessLocalMaxHeight() {
  //return m_JunctionnessLocalMaxFilter->GetHeight();
  return 0;
}


void 
DataModel
::SetImageData(TImage::Pointer image) {
  m_ImageData = image;

  // Connect the image data to the resampling filter and hook up the resampling
  // filter to the rest of the filters.
  m_ResampleFilter->SetInput(m_ImageData);
  m_ResampleFilter->SetSize(m_ImageData->GetLargestPossibleRegion().GetSize());
  m_ResampleFilter->SetOutputSpacing(m_ImageData->GetSpacing());
  m_ResampleFilter->UpdateLargestPossibleRegion();

  // Send the original image data to VTK
  m_InputImageITKToVTKFilter->SetInput(m_SquishZSpacingFilter->GetOutput());

  // Set filtered image data to input image initially.
  m_FilteredImageITKToVTKFilter->SetInput(m_SquishZSpacingFilter->GetOutput());
}


DataModel::TImage::Pointer
DataModel
::GetImageData() {
  return m_ImageData;
}


vtkAlgorithmOutput*
DataModel
::GetImageOutputPort() {
  return m_InputImageITKToVTKFilter->GetOutputPort();
}


vtkAlgorithmOutput*
DataModel
::GetFilteredImageOutputPort() {
  return m_FilteredImageITKToVTKFilter->GetOutputPort();
}


vtkAlgorithmOutput*
DataModel
::GetVectorOutputPort() {
  return m_VectorImageITKToVTKFilter->GetOutputPort();
}


double
DataModel
::GetFilteredDataMinimum() {
  return m_MinMaxFilter->GetMinimum();
}


double
DataModel
::GetFilteredDataMaximum() {
  return m_MinMaxFilter->GetMaximum();
}


void
DataModel
::GetDimensions(int dimensions[3]) {
  if (!GetImageData()) {
    dimensions[0] = 0;
    dimensions[1] = 0;
    dimensions[2] = 0;
    return;
  }

  UShort3DImageType::RegionType region 
      = GetImageData()->GetLargestPossibleRegion();
  itk::Size<3> size = region.GetSize();

  dimensions[0] = size[0];
  dimensions[1] = size[1];
  dimensions[2] = size[2];
}


void
DataModel
::GetResampledDimensions(int dimensions[3]) {
  if (!GetImageData()) {
    dimensions[0] = 0;
    dimensions[1] = 0;
    dimensions[2] = 0;
    return;
  }

  for (int i = 0; i < 3; i++) {
    dimensions[i] = m_ResampleFilter->GetSize()[i];
  }
}


void
DataModel
::SetFilterToNone() {
  if (m_ImageData) {
    m_MinMaxFilter->SetImage(m_ImageData);
    m_MinMaxFilter->Compute();
    
    m_FilteredImageITKToVTKFilter->SetInput(m_ImageData);
  }
}


void
DataModel
::SetFilterToFrangiFiberness() {
  if (!m_ImageData)
    return;

  m_FibernessFilter->Update();
  m_MinMaxFilter->SetImage(m_FibernessFilter->GetOutput());
  m_MinMaxFilter->Compute();
  m_FilteredImageITKToVTKFilter->SetInput(m_FibernessFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleFiberness() {
  if (!m_ImageData)
    return;

  m_MultiscaleFibernessFilter->Update();
  m_MinMaxFilter->SetImage(m_MultiscaleFibernessFilter->GetOutput());
  m_MinMaxFilter->Compute();
  m_FilteredImageITKToVTKFilter->SetInput(m_MultiscaleFibernessFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleFibernessThreshold() {
  if (!m_ImageData)
    return;

  m_MultiscaleFibernessThresholdFilter->Update();
  m_MinMaxFilter->SetImage(m_MultiscaleFibernessThresholdFilter->GetOutput());
  m_MinMaxFilter->Compute();
  m_FilteredImageITKToVTKFilter->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());
}


void
DataModel
::SetFilterToMultiscaleSkeletonization() {
  if (!m_ImageData)
    return;

  m_SkeletonizationFilter->Update();
  m_FilteredImageITKToVTKFilter->SetInput(m_SkeletonizationFilter->GetOutput());
}


void
DataModel
::SetFilterToJunctionness() {
  if (!m_ImageData)
    return;

  m_JunctionnessFilter->Update();
  m_MinMaxFilter->SetImage(m_JunctionnessFilter->GetOutput());
  m_MinMaxFilter->Compute();
  m_FilteredImageITKToVTKFilter->SetInput(m_JunctionnessFilter->GetOutput());
  m_VectorImageITKToVTKFilter->SetInput(
    m_JunctionnessFilter->GetEigenVectorInput());
}


void
DataModel
::SetFilterToJunctionnessLocalMax() {
  if (!m_ImageData)
    return;
  
  m_JunctionnessLocalMaxFilter->Update();
  m_MinMaxFilter->SetImage(m_JunctionnessFilter->GetOutput());
  m_MinMaxFilter->Compute();
  m_FilteredImageITKToVTKFilter->SetInput(m_JunctionnessLocalMaxFilter->GetOutput());
}


void
DataModel
::SetVoxelSpacing(double spacing[3]) {
  if (!m_ImageData)
    return;

  // All spacing changes go to the input image.
  m_ImageData->SetSpacing(spacing);

  // Then we update the resampled image, making the assumption that the size
  // of the voxels in the resampled image is cubic and matches the smallest
  // spacing in the input image.
  double minSpacing = spacing[0];
  for (int i = 1; i < 3; i++) {
    if (spacing[i] < minSpacing) minSpacing = spacing[i];
  }
  ResampleFilterType::SpacingType resampledSpacing;
  for (int i = 0; i < 3; i++)
    resampledSpacing[i] = minSpacing;
  
  // Calculate number of voxels in each dimension for the resampled image.
  InputImageType::SizeType size = 
    m_ImageData->GetLargestPossibleRegion().GetSize();

  ResampleFilterType::SizeType resampledSize;
  for (int i = 0; i < 3; i++) {
    resampledSize[i] = static_cast<unsigned int>((size[i] * spacing[i]) / minSpacing);
  }

  // Now set the size and spacing for the resampling filter.
  m_ResampleFilter->SetSize(resampledSize);
  m_ResampleFilter->SetOutputSpacing(resampledSpacing);

  m_InputImageITKToVTKFilter->Update();
  m_FilteredImageITKToVTKFilter->Update();
}


void
DataModel
::SetVoxelSpacing(int dimension, double spacing) {
  if (!m_ImageData)
    return;

  double currentSpacing[3];
  GetVoxelSpacing(currentSpacing);
  currentSpacing[dimension] = spacing;
  SetVoxelSpacing(currentSpacing);
}


void
DataModel
::SetVoxelXSpacing(double spacing) {
  SetVoxelSpacing(0, spacing); 
}


void
DataModel
::SetVoxelYSpacing(double spacing) {
  SetVoxelSpacing(1, spacing); 
}


void
DataModel
::SetVoxelZSpacing(double spacing) {
  SetVoxelSpacing(2, spacing); 
}


void
DataModel
::GetVoxelSpacing(double spacing[3]) {
  if (!GetImageData()) {
    spacing[0] = 0.0;
    spacing[1] = 0.0;
    spacing[2] = 0.0;
    return;
  }

  itk::Vector<double> thisSpacing = GetImageData()->GetSpacing();
  spacing[0] = thisSpacing[0];
  spacing[1] = thisSpacing[1];
  spacing[2] = thisSpacing[2];
}


void
DataModel
::GetResampledVoxelSpacing(double spacing[3]) {
  if (!GetImageData()) {
    spacing[0] = 0.0;
    spacing[1] = 0.0;
    spacing[2] = 0.0;
    return;
  }

  ResampleFilterType::SpacingType thisSpacing = 
    m_ResampleFilter->GetOutputSpacing();
  spacing[0] = thisSpacing[0];
  spacing[1] = thisSpacing[1];
  spacing[2] = thisSpacing[2];
}


void
DataModel
::GetImageCenter(double center[3]) {
  double spacing[3];
  GetVoxelSpacing(spacing);
  int dimensions[3];
  GetDimensions(dimensions);

  for (int i = 0; i < 3; i++) {
    center[i] = 0.5*(spacing[i]*static_cast<double>(dimensions[i]-1));
  }
}


double
DataModel
::GetMaxImageSize() {
  double spacing[3];
  GetVoxelSpacing(spacing);
  int dimensions[3];
  GetDimensions(dimensions);  

  double maxSize = 0.0;
  for (int i = 0; i < 3; i++) {
    double size = spacing[i] * static_cast<double>(dimensions[i]-1);
    if (size > maxSize)
      maxSize = size;
  }

  return maxSize;
}


void
DataModel
::SetReferenceDirectionAzimuth(double azimuth) {
  m_RefDirectionAzimuth = azimuth;
}


double
DataModel
::GetReferenceDirectionAzimuth() {
  return m_RefDirectionAzimuth;
}


void
DataModel
::SetReferenceDirectionInclination(double inclination) {
  m_RefDirectionInclination = inclination;
}


double
DataModel
::GetReferenceDirectionInclination() {
  return m_RefDirectionInclination;
}


void
DataModel
::SetZSquishFactor(double squishFactor) {
  m_ZSquishFactor = squishFactor;

  double spacing[3];
  GetResampledVoxelSpacing(spacing);

  spacing[2] *= m_ZSquishFactor;
  m_SquishZSpacingFilter->SetOutputSpacing(spacing);
}


double
DataModel
::GetZSquishFactor() {
  return m_ZSquishFactor;
}


void
DataModel
::SetFilteredImageScaleFactor(double scale) {
  m_FilteredImageScale = scale;
}


double
DataModel
::GetFilteredImageScaleFactor() {
  return m_FilteredImageScale;
}


void
DataModel
::ComputeConnectedComponentsVsThresholdData(double minThreshold, double maxThreshold,
                                             double thresholdIncrement, std::string fileName) {
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "thresholdValue\tconnectedComponents\n");

  for (double thresholdValue = minThreshold; thresholdValue <= maxThreshold; thresholdValue += thresholdIncrement) {
    
    // Compute number of connected components.
    m_MultiscaleFibernessThresholdFilter->SetLowerThreshold(0);
    m_MultiscaleFibernessThresholdFilter->SetUpperThreshold(thresholdValue);
    m_MultiscaleFibernessThresholdFilter->Modified();

    try {
      m_FibernessConnectedComponentsFilter->Update();
    } catch (...) {
      std::cout << "Exception caught in DataModel::ComputeConnectedComponentsVsThresholdData" << std::endl;
    }
    
    m_MinMaxConnectedComponentsFilter->SetImage(m_FibernessConnectedComponentsFilter->GetOutput());
    m_MinMaxConnectedComponentsFilter->Compute();
    int connectedComponents = m_MinMaxConnectedComponentsFilter->GetMaximum();

    printf("%f\t%d\n", thresholdValue, connectedComponents);
    fprintf(fp, "%f\t%d\n", thresholdValue, connectedComponents);
  }

  fclose(fp);
}


void
DataModel
::ComputeVolumeFractionEstimateVsZData(double threshold, std::string fileName) {
  FILE *fp = fopen(fileName.c_str(), "w");
  fprintf(fp, "z\tvolumeFraction\n");

  m_MultiscaleFibernessThresholdFilter->SetLowerThreshold(0);
  m_MultiscaleFibernessThresholdFilter->SetUpperThreshold(threshold);
  m_MultiscaleFibernessThresholdFilter->Modified();

  itk::AccumulateImageFilter< TImage, TImage >::Pointer accumulateFilter1 =
    itk::AccumulateImageFilter< TImage, TImage >::New();
  accumulateFilter1->SetInput(m_MultiscaleFibernessThresholdFilter->GetOutput());
  accumulateFilter1->SetAccumulateDimension(0);

  itk::AccumulateImageFilter<TImage, TImage>::Pointer accumulateFilter2 =
    itk::AccumulateImageFilter<TImage, TImage>::New();
  accumulateFilter2->SetInput(accumulateFilter1->GetOutput());
  accumulateFilter2->SetAccumulateDimension(1);
  accumulateFilter2->Update();

  TImage::RegionType accumulatedRegion 
    = accumulateFilter2->GetOutput()->GetLargestPossibleRegion();
  itk::Size<3> accumulatedRegionSize = accumulatedRegion.GetSize();

  printf("Reduced dimensions: %ld, %ld, %ld\n",
    accumulatedRegionSize[0], accumulatedRegionSize[1], accumulatedRegionSize[2]);

  // Compute number of pixels above the fiberness threshold value.
  int dims[3];
  GetDimensions(dims);

  TImage::PixelType pixelsPerSlice 
    = static_cast< TImage::PixelType >(dims[0]*dims[1]);
  printf("Pixels per slice: %f\n", pixelsPerSlice);

  TImage::Pointer image = accumulateFilter2->GetOutput();
  for (int i = 0; i < dims[2]; i++) {
    TImage::IndexType index;
    index[0] = 0;
    index[1] = 0;
    index[2] = i;
    TImage::PixelType sum = image->GetPixel(index) / pixelsPerSlice;
    fprintf(fp, "%d\t%f\n", i, sum);
  }

  fclose(fp);
}


void
DataModel
::SetProgressCallback(ProgressCallback callback) {
  progressCallback = callback;
}


void
DataModel
::UpdateProgress(itk::Object* object, const itk::EventObject& event) {
  itk::ProcessObject* processObject = dynamic_cast<itk::ProcessObject*>(object);
  if (progressCallback) {
    std::string filterName;
    itk::ExposeMetaData(object->GetMetaDataDictionary(), "filterName", filterName);
    std::string message("Running ");
    message.append(filterName);
    progressCallback(processObject->GetProgress(), message.c_str());
    std::cout << message << ": " 
      << processObject->GetProgress() << std::endl;
  }
}


#endif // _DATA_MODEL_CXX_
