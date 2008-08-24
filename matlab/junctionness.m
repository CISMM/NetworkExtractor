% Computes junctionness of pixels in a fiber. Returns an image where
% the value in the pixel is the number of fibers crossing a circle of 
% a certain radius centered at that pixel. 
%
% ARGUMENTS
%   image        - Original image of a network.
%   segmentation - Binary mask of the network.
%   radius       - Radius of junction probe to be used.
%   pctThreshold - Standard deviations above which the radial profile is
%                  considered to have a peak.
%
% OUTPUT
%   output       - Image whose pixels contain the number of fibers that
%                  cross the radial probe centered at the pixel location.

Segmentation is a binary image from fiber
% segmentation.
function output = junctionness(image, segmentation, radius, pctThreshold)

s = size(image);

divs = 64;
theta = 0:pi/divs:2*pi-(pi/divs);
iRadius = ceil(radius);
[x y] = meshgrid(1:s(2), 1:s(1));

output = zeros(size(image));
for j=iRadius:s(1)-iRadius
    for i=iRadius:s(2)-iRadius
        if segmentation(j,i) == 0
            continue;
        end
        
        interpx = radius*cos(theta) + i;
        interpy = radius*sin(theta) + j;
        
        vals = interp2(x, y, image, interpx, interpy, 'linear') .* ...
            interp2(x, y, segmentation, interpx, interpy, 'linear');
        
        % Find mean of radial profile values
        profileMean = mean(vals);
        profileStd  = std(vals);
        
        % Binarize the profile according to threshold
        binaryVals = vals > (profileMean + (profileStd * pctThreshold));
        crossings = diff([binaryVals binaryVals(1)]);
        spans = find(crossings > 0) - find(crossings < 0);

        % Look for fiber crossings.
        fiberWidth = 3.0 / ((2*pi/divs)*radius);
        output(j,i) = sum(abs(spans) > fiberWidth);
        
    end
end

