#include "FibrinAnalysisGUI.h"

#include "wxUtils.h"

using nanoCommon::std2wx;

static const std::string APP_NAME("FibrinAnalysis");
static const std::string APP_VERSION("v 0.1");
static const std::string APP_AUTHOR("Cory Quammen");
static const std::string APP_COPYRIGHT("(c) 2008");

static const std::string APP_WEBSITE("http://www.cs.unc.edu/Research/nano/cismm");

bool FibrinAnalysisApp::OnInit() {
  FibrinAnalysisGUI* fa = new FibrinAnalysisGUI((wxWindow*) NULL, -1, std2wx(APP_NAME + " " + APP_VERSION));
  fa->Show(true);
  SetTopWindow(fa);
  return true;
}

BEGIN_EVENT_TABLE(FibrinAnalysisGUI, wxFrame)
  EVT_CLOSE(FibrinAnalysisGUI::OnCloseWindow)
  EVT_MENU(MENU_EXIT, FibrinAnalysisGUI::OnExit)
END_EVENT_TABLE();


FibrinAnalysisGUI::FibrinAnalysisGUI(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
  : wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE) {

  m_destroyed = false;
  this->CreateWidgets();
  this->SetProperties();

  m_renderer = vtkRenderer::New();
  m_renderWindow = vtkRenderWindow::New();
  m_renderWindow->AddRenderer(m_renderer);

  m_rwiView = new wxVTKRenderWindowInteractor(this, -1);
  m_rwiView->SetRenderWindow(m_renderWindow);

  vtkInteractorStyleSwitch* iStyle = vtkInteractorStyleSwitch::New();
  iStyle->SetCurrentStyleToTrackballCamera();
  m_rwiView->SetInteractorStyle(iStyle);
  iStyle->Delete();

  vtkConeSource* coneSource = vtkConeSource::New();
  coneSource->SetResolution(16);

  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection(coneSource->GetOutputPort());
  coneSource->Delete();
  
  vtkActor* actor = vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();

  m_renderer->AddActor(actor);
  actor->Delete();
  
}


void FibrinAnalysisGUI::CreateWidgets() {
  m_menuBar = new wxMenuBar();
  SetMenuBar(m_menuBar);
  
  wxMenu* menuFile = new wxMenu();
  menuFile->Append(MENU_EXIT, wxT("E&xit"), wxT("Quit FibrinAnalysis"), wxITEM_NORMAL);
  m_menuBar->Append(menuFile, wxT("&File"));
  
  m_statusBar = CreateStatusBar(1, 0);
}


void FibrinAnalysisGUI::DoLayout() {
  wxBoxSizer *topLevelSizer = new wxBoxSizer(wxVERTICAL);
  SetAutoLayout(true);
  SetSizer(topLevelSizer);
  Layout();
}


void FibrinAnalysisGUI::SetProperties() {
  this->SetSize(wxSize(1000, 791));
  int statusBar_widths[] = { -1 };
  m_statusBar->SetStatusWidths(1, statusBar_widths);
  const wxString statusBar_fields[] = {
      wxT("Welcome to FibrinAnalysis!")
  };
  for(int i = 0; i < m_statusBar->GetFieldsCount(); ++i) {
    m_statusBar->SetStatusText(statusBar_fields[i], i);
  }
}


FibrinAnalysisGUI::~FibrinAnalysisGUI() {
}


bool FibrinAnalysisGUI::Destroy() { 
  if (!m_destroyed) {
    m_renderer->Delete();
    m_renderWindow->Delete();
    m_rwiView->Delete();
    wxFrame::Destroy();
  }
  m_destroyed = true;

  return true;
}


void FibrinAnalysisGUI::OnCloseWindow(wxCloseEvent &event) {
  this->Destroy();
}


void FibrinAnalysisGUI::OnExit(wxCommandEvent &event) {
  this->Destroy();
}