#ifndef _VISUALIZATION_PIPELINE_H_
#define _VISUALIZATION_PIPELINE_H_

class vtkAlgorithm;
class vtkAlgorithmOutput;
class vtkRenderer;

class VisualizationPipeline {

public:
  VisualizationPipeline();
  virtual ~VisualizationPipeline();

  void SetInputConnection(vtkAlgorithmOutput* input);

  virtual void AddToRenderer(vtkRenderer* renderer) = 0;
  
protected:
  vtkAlgorithm* inputAlgorithm;

  void SetInputAlgorithm(vtkAlgorithm* algorithm);
  
};

#endif _VISUALIZATION_PIPELINE_H_