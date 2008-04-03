#include <wx/wx.h>
#include <wx/image.h>
#include <wx/splitter.h>

#include "wxVTKRenderWindowInteractor.h"

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
    MENU_EXIT      = 1000,
  };

  FibrinAnalysisGUI(wxWindow* parent, int id, const wxString& title, 
    const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

  virtual ~FibrinAnalysisGUI();

  virtual bool Destroy();

  void OnCloseWindow(wxCloseEvent& event);
  void OnExit(wxCommandEvent& event);

protected:

  wxMenuBar* m_menuBar;
  wxStatusBar* m_statusBar;
  wxVTKRenderWindowInteractor* m_rwiView;

  DECLARE_EVENT_TABLE();

  void CreateWidgets();

private:

  bool m_destroyed;

  vtkRenderer* m_renderer;

  vtkRenderWindow *m_renderWindow;

  void DoLayout();

  void SetProperties();

};

#endif // _FIBRIN_ANALYSIS_GUI_H_
