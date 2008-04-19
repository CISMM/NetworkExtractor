#include "FibrinAnalysisGUI.h"

#include "wxUtils.h"

#include <sstream>

#include "itkImageFileReader.h"
#include "itkNumericSeriesFileNames.h"

#include "vtkImageImport.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageData.h"
#include "vtkContourFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"

using nano::std2wx;

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
  EVT_MENU(MENU_FILE_IMPORT,  FibrinAnalysisGUI::OnImport)
  EVT_MENU(MENU_FILE_OPEN,    FibrinAnalysisGUI::OnOpen)
  EVT_MENU(MENU_FILE_CLOSE,   FibrinAnalysisGUI::OnClose)
  EVT_MENU(MENU_FILE_SAVE,    FibrinAnalysisGUI::OnSave)
  EVT_MENU(MENU_FILE_SAVE_AS, FibrinAnalysisGUI::OnSaveAs)
  EVT_MENU(MENU_FILE_EXIT,    FibrinAnalysisGUI::OnExit)
  EVT_CLOSE(FibrinAnalysisGUI::OnCloseWindow)
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

  this->LoadAndDisplayImage();
  
}


void FibrinAnalysisGUI::CreateWidgets() {
  m_menuBar = new wxMenuBar();
  SetMenuBar(m_menuBar);
  
  wxMenu* menuFile = new wxMenu();
  menuFile->Append(MENU_FILE_IMPORT, wxT("Import"), wxT("Import image files"), wxITEM_NORMAL);
  menuFile->AppendSeparator();
  menuFile->Append(MENU_FILE_OPEN, wxT("&Open\tCtrl+O"), wxT("Open files"), wxITEM_NORMAL);
  menuFile->Append(MENU_FILE_CLOSE, wxT("Close"), wxT("Close"), wxITEM_NORMAL);
  menuFile->AppendSeparator();
  menuFile->Append(MENU_FILE_SAVE, wxT("&Save\tCtrl+S"), wxT("Save"), wxITEM_NORMAL);
  menuFile->Append(MENU_FILE_SAVE_AS, wxT("Save as...\tCtrl+Shift+S"), wxT("Save as..."), wxITEM_NORMAL);
  menuFile->AppendSeparator();
  menuFile->Append(MENU_FILE_EXIT, wxT("E&xit"), wxT("Quit FibrinAnalysis"), wxITEM_NORMAL);
  m_menuBar->Append(menuFile, wxT("&File"));
  
  m_statusBar = CreateStatusBar(1, 0);
}


void FibrinAnalysisGUI::LoadAndDisplayImage() {
  // Enumerate TIF files to load
  itk::NumericSeriesFileNames::Pointer fileNames = itk::NumericSeriesFileNames::New();
  fileNames->SetSeriesFormat("D:\\cquammen\\nano\\wolberg\\thick_stack\\tiff_stack_from_lsm\\utset.080305rc1%04d.tif");
  fileNames->SetStartIndex(1);
  fileNames->SetIncrementIndex(1);
  fileNames->SetEndIndex(50);

  SeriesReaderType::Pointer seriesReader = SeriesReaderType::New();
  seriesReader->SetFileNames(fileNames->GetFileNames());
  seriesReader->Update();
  
  // Set up Hessian filter in preparation for extracting principal directions of curvature
  HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
  hessianFilter->SetInput(seriesReader->GetOutput());

  VesselnessFilterType::Pointer vesselnessFilter = VesselnessFilterType::New();
  vesselnessFilter->SetInput(hessianFilter->GetOutput());
  vesselnessFilter->Update();

  m_exporter = ExporterType::New();
  m_exporter->SetInput(vesselnessFilter->GetOutput());
  m_exporter->Update();

  ImageType::Pointer image = vesselnessFilter->GetOutput();
  std::ostringstream msg("");
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  msg << std::string("Dimensions: ") << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2];

  vtkImageImport* imageImporter = vtkImageImport::New();
  imageImporter->SetUpdateInformationCallback(m_exporter->GetUpdateInformationCallback());
  imageImporter->SetPipelineModifiedCallback(m_exporter->GetPipelineModifiedCallback());
  imageImporter->SetWholeExtentCallback(m_exporter->GetWholeExtentCallback());
  imageImporter->SetSpacingCallback(m_exporter->GetSpacingCallback());
  imageImporter->SetOriginCallback(m_exporter->GetOriginCallback());
  imageImporter->SetScalarTypeCallback(m_exporter->GetScalarTypeCallback());
  imageImporter->SetNumberOfComponentsCallback(m_exporter->GetNumberOfComponentsCallback());
  imageImporter->SetPropagateUpdateExtentCallback(m_exporter->GetPropagateUpdateExtentCallback());
  imageImporter->SetUpdateDataCallback(m_exporter->GetUpdateDataCallback());
  imageImporter->SetDataExtentCallback(m_exporter->GetDataExtentCallback());
  imageImporter->SetBufferPointerCallback(m_exporter->GetBufferPointerCallback());
  imageImporter->SetCallbackUserData(m_exporter->GetCallbackUserData());

  vtkVolumeTextureMapper3D *textureMapper = vtkVolumeTextureMapper3D::New();
  textureMapper->SetInputConnection(imageImporter->GetOutputPort());
  imageImporter->Delete();
  imageImporter->Update();

  double *range = imageImporter->GetOutput()->GetScalarRange();
  msg << ": " << range[0] << ", " << range[1];
  m_statusBar->SetStatusText(std2wx(msg.str()));

  //vtkVolume *volume = vtkVolume::New();
  //volume->SetMapper(textureMapper);
  //textureMapper->Delete();

  //vtkPiecewiseFunction *opacityMap = vtkPiecewiseFunction::New();
  ////opacityMap->AddSegment(0.0, 0.0, 10.0, 0.0);
  ////opacityMap->AddSegment(10.0, 0.0, 25.0, 1.0);
  ////opacityMap->AddSegment(25.0, 1.0, 255.0, 1.0);
  //opacityMap->AddSegment(0.0, 0.0, 5.0, 0.0);
  //opacityMap->AddSegment(5.0, 0.0, 10.0, 1.0);
  //opacityMap->AddSegment(10.0, 1.0, 255.0, 1.0);

  //vtkColorTransferFunction *tfunc = vtkColorTransferFunction::New();
  //tfunc->AddRGBSegment(0.0, 1.0, 1.0, 1.0, 255.0, 1.0, 1.0, 1.0);

  //vtkVolumeProperty *vProp = vtkVolumeProperty::New();
  //vProp->SetScalarOpacity(opacityMap);
  //opacityMap->Delete();
  //vProp->SetColor(tfunc);
  //tfunc->Delete();
  //vProp->SetInterpolationTypeToLinear();
  //vProp->ShadeOn();

  //volume->SetProperty(vProp);
  //vProp->Delete();

  //m_renderer->AddVolume(volume);
  //volume->Delete();

  vtkContourFilter *contourer = vtkContourFilter::New();
  contourer->SetInputConnection(imageImporter->GetOutputPort());
  contourer->SetNumberOfContours(1);
  contourer->SetValue(0, 5.0);

  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection(contourer->GetOutputPort());
  mapper->ScalarVisibilityOff();
  
  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);

  m_renderer->AddActor(actor);

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


void FibrinAnalysisGUI::OnImport(wxCommandEvent& event) {
  m_statusBar->SetStatusText(wxT("Imported image files"));
}


void FibrinAnalysisGUI::OnOpen(wxCommandEvent& event) {
  m_statusBar->SetStatusText(wxT("Opened file"));
}


void FibrinAnalysisGUI::OnClose(wxCommandEvent& event) {
  m_statusBar->SetStatusText(wxT("Closed file"));
}


void FibrinAnalysisGUI::OnSave(wxCommandEvent& event) {
  m_statusBar->SetStatusText(wxT("Saved file"));
}


void FibrinAnalysisGUI::OnSaveAs(wxCommandEvent& event) {
  m_statusBar->SetStatusText(wxT("Saved file as"));
}


void FibrinAnalysisGUI::OnExit(wxCommandEvent& event) {
  this->Destroy();
}


void FibrinAnalysisGUI::OnCloseWindow(wxCloseEvent& event) {
  this->Destroy();
}
