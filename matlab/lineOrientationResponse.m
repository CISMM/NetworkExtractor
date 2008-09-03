% Rotates the Gaussian template around a point in an image and calculates
% the response to an ideal Gaussian template.
%
% ARGUMENTS
%   image      - The image containing the line structure.
%   pt         - Position that serves as an anchor for one end of the line.
%   sigma      - Scale of line interrogation.
%   radius     - Length of line filter.
%   angleStart - Starting angle in range to check (in degrees).
%   angleEnd   - Ending angle in range to check (in degrees).
%   angleIncr  - Angle increment during search.
%   
% OUTPUT
%   response - Filter response over the range of angles specified.
%   angles   - Angles corresponding to the filter responses.

function [response angles] = lineOrientationResponse(image, pt, sigma, radius, angleStart, angleEnd, angleIncr)
    angles = angleStart:angleIncr:angleEnd;
    numAngles = length(angles);
    response = zeros(numAngles,1);
    
    % Compute bounds of image patch to be examined
    offset = pt - floor(pt);
    patchDim = size(orientedGaussianLine(offset, radius, 0, sigma), 1);
    patchHalfDim = floor(patchDim/2);
    
    % Store image patch in temporary variable.
    ipt = floor(pt);
    ix = ipt(1);
    iy = ipt(2);
    startX = ix - patchHalfDim;
    endX   = ix + patchHalfDim;
    startY = iy - patchHalfDim;
    endY   = iy + patchHalfDim;
    patch = image(startY:endY, startX:endX);
    patchNormalized  = patch - mean(mean(patch));
    patchStdDev = std(reshape(patch, [prod(size(patch)) 1]));
    
    for i = 1:numAngles
        theta = angles(i);

        % Generate Gaussian line
        filter = orientedGaussianLine(offset, radius, theta, sigma);
        
        % Compute normalized cross-correlation between patch and filter
        filterNormalized = filter - mean(mean(filter));
        filterStdDev = std(reshape(filter, [prod(size(patch)) 1]));
        response(i) = sum(sum(patchNormalized .* filterNormalized)) /   ...
                      (patchStdDev * filterStdDev);
        
        figure(1);
        imagescale((patchNormalized - filterNormalized));        
    end
    
    figure(2);
    plot(angles, response);

end