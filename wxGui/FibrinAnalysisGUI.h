#include <wx/wx.h>
#include <wx/image.h>
#include <wx/splitter.h>
#include "wxVTKRenderWindowInteractor.h"

#include "itkImageSeriesReader.h"
#include "itkSymmetricSecondRankTensor.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkVTKImageExport.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkInteractorStyleSwitch.h"


#ifndef _FIBRIN_ANALYSIS_GUI_H_
#define _FIBRIN_ANALYSIS_GUI_H_


class FibrinAnalysisApp : public wxApp {
public:
  virtual bool OnInit();
};

class FibrinAnalysisGUI : public wxFrame {
public:

  enum IDs {
    MENU_FILE_IMPORT = 1000,
    MENU_FILE_OPEN,
    MENU_FILE_CLOSE,
    MENU_FILE_SAVE,
    MENU_FILE_SAVE_AS,
    MENU_FILE_EXIT
  };

  FibrinAnalysisGUI(wxWindow* parent, int id, const wxString& title, 
    const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

  virtual ~FibrinAnalysisGUI();

  virtual bool Destroy();

  void OnImport(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnSaveAs(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnCloseWindow(wxCloseEvent& event);

protected:

  typedef unsigned short PixelType;
  static const unsigned int     Dimension = 3;
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef itk::ImageSeriesReader<ImageType> SeriesReaderType;
  typedef itk::SymmetricSecondRankTensor<double, 3> HessianType;
  typedef itk::HessianRecursiveGaussianImageFilter<ImageType> HessianFilterType;
  typedef itk::Hessian3DToVesselnessMeasureImageFilter<PixelType> VesselnessFilterType;
  typedef itk::VTKImageExport<ImageType> ExporterType;

  ExporterType::Pointer m_exporter;

  wxMenuBar* m_menuBar;
  wxStatusBar* m_statusBar;
  wxVTKRenderWindowInteractor* m_rwiView;

  DECLARE_EVENT_TABLE();

  void CreateWidgets();

  void LoadAndDisplayImage();

private:

  bool m_destroyed;

  vtkRenderer* m_renderer;

  vtkRenderWindow *m_renderWindow;

  void DoLayout();

  void SetProperties();

};

#endif // _FIBRIN_ANALYSIS_GUI_H_
