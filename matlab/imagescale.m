% Replacement for imshow that scales a grayscale image's intensity range to
% the display range [0,255] automatically. Shows image with rescaled 
% intensities in a MATLAB figure window.
%
% ARGUMENTS
%   image    - Image to display
%   varargin - Optional list of arguments. In this case, the optional
%              argument is a scaling factor to be applied to intensities
%              after they have been converted to the range [0,255]. This is
%              useful for highlighting darker parts of images.
%
% OUTPUT
%   h        - Handle to the figure window

function h = imagescale(image, varargin)

brightnessFactor = 1.0;

if (length(varargin) > 0)
    brightnessFactor = varargin{1};
end

range = [min(min(image)) max(max(image)) / brightnessFactor];
if (min(range) == max(range))
    range(2) = 1e-4;
end

h = imshow(image, range, 'Border', 'tight');