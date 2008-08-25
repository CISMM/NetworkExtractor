function varargout = fibrinGUI(varargin)
% fibrinGUI M-file for fibrinGUI.fig
%      fibrinGUI, by itself, creates a new fibrinGUI or raises the existing
%      singleton*.
%
%      H = fibrinGUI returns the handle to a new fibrinGUI or the handle to
%      the existing singleton*.
%
%      fibrinGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in fibrinGUI.M with the given input arguments.
%
%      fibrinGUI('Property','Value',...) creates a new fibrinGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before fibrinGUI_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to fibrinGUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help fibrinGUI

% Last Modified by GUIDE v2.5 24-Aug-2008 18:52:00

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @fibrinGUI_OpeningFcn, ...
                   'gui_OutputFcn',  @fibrinGUI_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before fibrinGUI is made visible.
function fibrinGUI_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to fibrinGUI (see VARARGIN)

% Choose default command line output for fibrinGUI
handles.output = hObject;

% Open image data here.
handles.showMaskedImage = 0;
handles.invertMask      = 0;
handles.showJunctions   = 0;
handles.showSkeleton    = 0;
handles.junctionAlgorithm = 'shape + curvedness';

% Update handles structure
guidata(hObject, handles);

% Show original image initially
%handles = refreshGUI(hObject, handles);
%refreshImage(hObject, handles);

% UIWAIT makes fibrinGUI wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Opens a file for analysis and processing
function handles = openImage(hObject, handles, filePath)
% Load image file
image = double(imread(filePath));
%image = medfilt2(image, [3 3]);
imageSize = size(image);

% Convert from color to gray scale if necessary
if length(imageSize) == 3
    image = (0.2989*image(:,:,1)) + (0.587*image(:,:,2)) + (0.114*image(:,:,3));
end
handles.image = image;

% Perform curvatures calculation and store the data
sigma = 1.0;
[k1 k2 shape curvedness px py] = curvatures2(handles.image, sigma);
handles.k1 = -k1;
handles.k2 = -k2;
handles.shape = shape;
handles.curvedness = curvedness;
handles.px = px;
handles.py = py;
handles.gradient = gradientMagnitude(handles.image, sigma);
handles.brightnessFactor = 1.0;
handles.displayImage = handles.image;
handles.maskMin = min(min(handles.displayImage));
handles.maskMax = max(max(handles.displayImage));
guidata(hObject, handles);


% --------------------------------------------------------------------
function saveMaskImage_Callback(hObject, eventdata, handles)
% hObject    handle to saveMaskImage (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
maskMin = handles.maskMin;
maskMax = handles.maskMax;
aboveMinImage = handles.displayImage >= maskMin;
belowMaxImage = handles.displayImage <= maskMax;
maskImage = aboveMinImage .* belowMaxImage;

fileSpec = {'*.jpg;*.gif;*.tif;*.png','All Image Files'};
titleString = 'Save Image File';
[filename pathname filterindex] = uiputfile(fileSpec, titleString);
if filename ~= 0
    filePath = sprintf('%s%s', pathname, filename);
    imwrite(maskImage, filePath);
end
guidata(hObject, handles);

% --------------------------------------------------------------------
function saveSkeletonImage_Callback(hObject, eventdata, handles)
% hObject    handle to saveSkeletonImage (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Outputs from this function are returned to the command line.
function varargout = fibrinGUI_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


function curvednessEdit_Callback(hObject, eventdata, handles)
% hObject    handle to curvednessEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of curvednessEdit as text
%        str2double(get(hObject,'String')) returns contents of curvednessEdit as a double
set(handles.valueSlider, 'Value', str2double(handles.curvednessThreshold));
valueSlider_Callback(handles.valueSlider, eventdata, handles);
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function curvednessEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to curvednessEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
guidata(hObject, handles);


% --- Executes on selection change in imagePopupMenu.
function imagePopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to imagePopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns imagePopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from
%        imagePopupMenu
contents = get(hObject,'String');
handles.displayImage = handles.image;
switch contents{get(hObject,'Value')}
    case 'image'
        handles.displayImage = handles.image;
        
    case 'gradient'
        handles.displayImage = handles.gradient;
        
    case 'k1'
        handles.displayImage = handles.k1;
    
    case 'k2'
        handles.displayImage = handles.k2;
        
    case 'shape'
        handles.displayImage = handles.shape;
        
    case 'curvedness'
        handles.displayImage = handles.curvedness;
        
    otherwise
        handles.displayImage = handles.image;
end

handles = refreshGUI(hObject, handles);
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function imagePopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to imagePopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

handles.imagePopupMenu = hObject;
guidata(hObject, handles);


% --- Updates GUI widgets with values from the currently selected image.
function handles = refreshGUI(hObject, handles)
minimum = min(min(handles.displayImage));
maximum = max(max(handles.displayImage));
set(handles.minEdit, 'String', num2str(minimum));
set(handles.maxEdit, 'String', num2str(maximum));
set(handles.maskMinEdit, 'String', num2str(minimum));
set(handles.maskMaxEdit, 'String', num2str(maximum));
handles.maskMin = minimum;
handles.maskMax = maximum;
guidata(hObject, handles);


% --- Updates image display, displaying only pixels within the mask range.
function handles = refreshImage(hObject, handles)
maskMin = handles.maskMin;
maskMax = handles.maskMax;
aboveMinImage = handles.displayImage >= maskMin;
belowMaxImage = handles.displayImage <= maskMax;
maskImage = aboveMinImage .* belowMaxImage;

if handles.invertMask
    maskIndices = find(maskImage == 0);
    handles.inverseMaskIndices = find(maskImage == 1);
else
    maskIndices = find(maskImage == 1);
    handles.inverseMaskIndices = find(maskImage == 0);
end

if handles.showMaskedImage == 0
    displayImage = min(min(handles.displayImage)) * ones(size(handles.displayImage));
    displayImage(maskIndices) = handles.displayImage(maskIndices);
else
    displayImage = min(min(handles.image)) * ones(size(handles.image));
    displayImage(maskIndices) = handles.image(maskIndices);
end
axes(handles.imageAxes)
imagescale(displayImage, handles.brightnessFactor);

if handles.showSkeleton ~= 0
   % Skeletonize the mask and show the results.
   skeleton = bwmorph(maskImage, 'skel', Inf);
   hold on;
   imagescale(skeleton);
   hold off;
end

guidata(hObject, handles);

% Update histogram
axes(handles.histogramAxes)
[n, xout] = hist(reshape(handles.displayImage, [prod(size(handles.displayImage)) 1]), 50);
bar(xout, log(n));


function minEdit_Callback(hObject, eventdata, handles)
% hObject    handle to minEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of minEdit as text
%        str2double(get(hObject,'String')) returns contents of minEdit as a double


% --- Executes during object creation, after setting all properties.
function minEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to minEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
handles.minEdit = hObject;
guidata(hObject, handles);


function maxEdit_Callback(hObject, eventdata, handles)
% hObject    handle to maxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of maxEdit as text
%        str2double(get(hObject,'String')) returns contents of maxEdit as a double


% --- Executes during object creation, after setting all properties.
function maxEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to maxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
handles.maxEdit = hObject;
guidata(hObject, handles);


function maskMinEdit_Callback(hObject, eventdata, handles)
% hObject    handle to maskMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of maskMinEdit as text
%        str2double(get(hObject,'String')) returns contents of maskMinEdit as a double
handles.maskMin = str2double(get(hObject,'String'));
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function maskMinEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to maskMinEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
handles.maskMinEdit = hObject;
guidata(hObject, handles);


function maskMaxEdit_Callback(hObject, eventdata, handles)
% hObject    handle to maskMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of maskMaxEdit as text
%        str2double(get(hObject,'String')) returns contents of maskMaxEdit as a double
handles.maskMax = str2double(get(hObject,'String'));
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function maskMaxEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to maskMaxEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
handles.maskMaxEdit = hObject;
guidata(hObject, handles);


function brightnessEdit_Callback(hObject, eventdata, handles)
% hObject    handle to brightnessEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of brightnessEdit as text
%        str2double(get(hObject,'String')) returns contents of brightnessEdit as a double
handles.brightnessFactor = str2double(get(hObject,'String'));
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function brightnessEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to brightnessEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in maskImageCheckbox.
function maskImageCheckbox_Callback(hObject, eventdata, handles)
% hObject    handle to maskImageCheckbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of maskImageCheckbox
handles.showMaskedImage = get(hObject,'Value');
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes on button press in invertMaskCheckbox.
function invertMaskCheckbox_Callback(hObject, eventdata, handles)
% hObject    handle to invertMaskCheckbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of invertMaskCheckbox
handles.invertMask = get(hObject,'Value');
handles = refreshImage(hObject, handles);
guidata(hObject, handles);


% --- Executes on button press in skeletonizeMask.
function skeletonizeMask_Callback(hObject, eventdata, handles)
% hObject    handle to skeletonizeMask (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of skeletonizeMask
handles.showSkeleton = get(hObject, 'Value');
refreshImage(hObject, handles);
guidata(hObject, handles)


% --- Executes on button press in overlayCheckbox.
function overlayCheckbox_Callback(hObject, eventdata, handles)
% hObject    handle to overlayCheckbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of overlayCheckbox
if get(hObject, 'Value') == 1
    handles = refreshImage(hObject, handles);
    maskIndices = handles.inverseMaskIndices;
    axes(handles.imageAxes);
    hold on;
    px = handles.px;
    py = handles.py;
    px(maskIndices) = 0;
    py(maskIndices) = 0;
    sparseQuiver(px, py, 2);
    hold off;
else
    refreshImage(hObject, handles);
end

guidata(hObject, handles);


% --------------------------------------------------------------------
function fileMenu_Callback(hObject, eventdata, handles)
% hObject    handle to fileMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function openMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to openMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
fileSpec = {'*.jpg;*.gif;*.tif;*.png','All Image Files'};
titleString = 'Load Image File';
[filename pathname filterindex] = uigetfile(fileSpec, titleString);
if filename ~= 0
    filePath = sprintf('%s%s', pathname, filename);
    handles = openImage(hObject, handles, filePath);
    handles = refreshGUI(hObject, handles);
    handles = refreshImage(hObject, handles);
    
    % Reset popup menu to 'image'
    set(handles.imagePopupMenu,'Value',1);
    
end
guidata(hObject, handles);


% --------------------------------------------------------------------
function exitMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to exitMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close(handles.figure1)



% --- Executes on selection change in algorithmPopupMenu.
function algorithmPopupMenu_Callback(hObject, eventdata, handles)
% hObject    handle to algorithmPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns algorithmPopupMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from algorithmPopupMenu
contents = get(hObject,'String');
handles.junctionAlgorithm = contents{get(hObject,'Value')};
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function algorithmPopupMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to algorithmPopupMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in showJunctionCheckbox.
function showJunctionCheckbox_Callback(hObject, eventdata, handles)
% hObject    handle to showJunctionCheckbox (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of showJunctionCheckbox
handles.showJunctions = get(hObject, 'Value');
if handles.showJunctions
    imageSize = size(handles.displayImage);
    x = [];
    y = [];
    
    handles.junctionAlgorithm
    switch handles.junctionAlgorithm
    
        case 'shape + curvedness'
            [x y] = shapePlusCurvedness(handles.shape, handles.curvedness);
            
        case 'shape + k2'
            [x y] = shapePlusK2(handles.shape, handles.k2);
        
        otherwise
            x = [];
            y = [];
    end
    
    % Plot junction points with boxes
    axes(handles.imageAxes);
    hold on;
    plot(x, y, 'sy');
    hold off;
else
    refreshImage(hObject, handles);
end

guidata(hObject, handles);
