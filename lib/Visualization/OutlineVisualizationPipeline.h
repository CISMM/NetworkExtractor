#ifndef _OUTLINE_VISUALIZATION_PIPELINE_H_
#define _OUTLINE_VISUALIZATION_PIPELINE_H_

#include "VisualizationPipeline.h"

#include <vtkActor.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>


class OutlineVisualizationPipeline : public VisualizationPipeline {

public:
	OutlineVisualizationPipeline();
	virtual ~OutlineVisualizationPipeline();

  virtual void AddToRenderer(vtkRenderer* renderer);

  void SetVisible(bool visible);
  bool GetVisible();

protected:
  vtkOutlineFilter*  m_OutlineFilter;
  vtkPolyDataMapper* m_OutlineMapper;
  vtkActor*          m_OutlineActor;

};


#endif // _OUTLINE_VISUALIZATION_PIPELINE_H_

