% This function filters a 2D image with 64 oriented line filters evenly
% spaced in the range [0,pi].
%
% ARGUMENTS
%   image            - 2D image of lines to be found
%   sigma            - scale at which to look for lines
%
% OUTPUT
%   sumImage         - image where each pixel contains the sum of the 
%                      responses from all oriented line filters
%   maxImage         - image where each pixel contains the maximum response
%                      from the oriented line filters
%   orientationImage - image where each pixel contains the orientation of 
%                      the line as determined by the maximum line filter
%                      response

function [sumImage maxImage orientationImage] = orientedLineFilter(image, sigma)

divisions = 64;

tmpKernel = Gvv2(sigma, 0);
maxImageSize = size(conv2(image, tmpKernel, 'valid'));

maxImage = -1e6*ones(maxImageSize);
sumImage = zeros(maxImageSize);
orientationImage = zeros(maxImageSize);
for theta=0:pi/divisions:pi-(pi/divisions)
    kernel = Gvv2(sigma, theta);
    currentImage = conv2(image, kernel, 'valid');
    sizeDiff = size(maxImage) - size(currentImage);
    currentImage = padarray(currentImage, sizeDiff/2);
    
    orientationImage(orientationImage > maxImage) = theta + (pi/2);
    
    maxImage = max(maxImage, currentImage);
    sumImage = sumImage + currentImage;
end

% Pad max, orientation, and sum images
padSize = (size(image) - size(maxImage)) / 2;
sumImage = padarray(sumImage, padSize);
maxImage = padarray(maxImage, padSize);
orientationImage = padarray(orientationImage, padSize);

%sumImage = sumImage ./ image;