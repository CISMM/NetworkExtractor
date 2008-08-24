% Draws a network of lines with Gaussian profile.
%
% ARGUMENTS
%   dimension - size in pixels of the image to be produced
%   sigma     - standard deviation of the Gaussian line profile
%   points    - list of 2D point positions, one per row
%   lines     - list of lines defined as indices into point position array,
%               one per row
%
% OUTPUT
%   output    - network image

function output = drawNetwork(dimension, sigma, points, lines)

output = zeros(dimension);

linesSize = size(lines);
for l=1:linesSize(1);
    p1 = points(lines(l,1),:);
    p2 = points(lines(l,2),:);
    
    % Get Gaussian-convolved line
    [lineImage offX offY] = gaussianLine(p1, p2, sigma);
    [sizeY sizeX] = size(lineImage);
    startX = offX;
    endX = startX + sizeX - 1;
    startY = offY;
    endY = startY + sizeY - 1;
    
    maxImage = output(startY:endY, startX:endX);
    
    % Put maximum of existing and new pixel values in the image
    greaterIndices = find(lineImage > maxImage);
    maxImage(greaterIndices) = lineImage(greaterIndices);
    output(startY:endY, startX:endX) = maxImage;
end