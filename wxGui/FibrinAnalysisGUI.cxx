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

  //vtkConeSource* coneSource = vtkConeSource::New();
  //coneSource->SetResolution(16);

  //vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  //mapper->SetInputConnection(coneSource->GetOutputPort());
  //coneSource->Delete();
  //
  //vtkActor* actor = vtkActor::New();
  //actor->SetMapper(mapper);
  //mapper->Delete();

  //m_renderer->AddActor(actor);
  //actor->Delete();

  this->LoadAndDisplayImage();
  
}


void FibrinAnalysisGUI::CreateWidgets() {
  m_menuBar = new wxMenuBar();
  SetMenuBar(m_menuBar);
  
  wxMenu* menuFile = new wxMenu();
  menuFile->Append(MENU_EXIT, wxT("E&xit"), wxT("Quit FibrinAnalysis"), wxITEM_NORMAL);
  m_menuBar->Append(menuFile, wxT("&File"));
  
  m_statusBar = CreateStatusBar(1, 0);
}


void FibrinAnalysisGUI::LoadAndDisplayImage() {
  // Enumerate TIF files to load
  itk::NumericSeriesFileNames::Pointer fileNames = itk::NumericSeriesFileNames::New();
  fileNames->SetSeriesFormat("D:\\cquammen\\nano\\wolberg\\thick_stack\\tiff_stack_from_lsm\\utset.080305rc1%04d.tif");
  fileNames->SetStartIndex(1);
  fileNames->SetIncrementIndex(1);
  fileNames->SetEndIndex(100);

  SeriesReaderType::Pointer seriesReader = SeriesReaderType::New();
  seriesReader->SetFileNames(fileNames->GetFileNames());
  seriesReader->Update();
  
  m_exporter = ExporterType::New();
  m_exporter->SetInput(seriesReader->GetOutput());
  m_exporter->Update();

  ImageType::Pointer image = seriesReader->GetOutput();

  std::ostringstream msg("");
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  msg << std::string("Dimensions: ") << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2];
  m_statusBar->SetStatusText(std2wx(msg.str()));

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

  vtkVolume *volume = vtkVolume::New();
  volume->SetMapper(textureMapper);
  textureMapper->Delete();

  vtkPiecewiseFunction *opacityMap = vtkPiecewiseFunction::New();
  opacityMap->AddSegment(0.0, 0.0, 10.0, 0.0);
  opacityMap->AddSegment(10.0, 0.0, 25.0, 1.0);
  opacityMap->AddSegment(25.0, 1.0, 255.0, 1.0);

  vtkColorTransferFunction *tfunc = vtkColorTransferFunction::New();
  tfunc->AddRGBSegment(0.0, 1.0, 1.0, 1.0, 255.0, 1.0, 1.0, 1.0);

  vtkVolumeProperty *vProp = vtkVolumeProperty::New();
  vProp->SetScalarOpacity(opacityMap);
  opacityMap->Delete();
  vProp->SetColor(tfunc);
  tfunc->Delete();
  vProp->SetInterpolationTypeToLinear();
  vProp->ShadeOn();

  volume->SetProperty(vProp);
  vProp->Delete();

  m_renderer->AddVolume(volume);
  volume->Delete();

  //typedef itk::ImageFileReader<ImageType>  ReaderType;
  //ReaderType::Pointer reader = ReaderType::New();

  //const std::string fileName = "D:\\cquammen\\nano\\wolberg\\thick_stack\\tiff_stack_from_lsm\\utset.080305rc10001.tif";
  //reader->SetFileName(fileName);
  //reader->Update();
  
  //ImageType::Pointer image = reader->GetOutput();

  //std::ostringstream msg("");
  //ImageType::RegionType region = image->GetLargestPossibleRegion();
  //msg << std::string("Dimensions: ") << region.GetSize()[0] << ", " << region.GetSize()[1];
  //

  //m_statusBar->SetStatusText(std2wx(msg.str()));
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