% This is an early attempt at finding junctions of lines in a network image
% by looking for things that do not have a tubular shape index but which
% have a bright background. It works okay for synthetic images, but not
% great for real images.
%
% ARGUMENTS
%   image     - Input image of network structure.
%   sigma     - Scale at which junction finding should take place.
%   tolerance - Tolerance specifying how far the shape index at a pixel can
%               be from cylindrical and still be considered cylindrical.
%   pctMax    - Threshold above which pixel values in the raw image are
%               considered part of the network image.

function [rawImage nodes] = junctions(image, sigma, tolerance, pctMax)

[k1 k2 shape] = curvatures2(image, sigma);

padded_shape = abs(padarray(shape, (size(image) - size(shape))/2));
F = zeros(size(image));
F(abs(padded_shape) - 0.5 > tolerance) = 1;

laplacian_image = conv2(image, -laplacian2(sigma), 'valid');
padded_laplacian = padarray(laplacian_image, (size(image) - size(laplacian_image))/2);

% Output the raw image
rawImage = F .* padded_laplacian;

% Now find the centroids of the potential junctions whose junction metrics
% are above some percentage of the value range in the raw image.
rawImageMin = min(min(rawImage));
rawImageMax = max(max(rawImage));
threshold = pctMax*(rawImageMax-rawImageMin) + rawImageMin;
[labeled, num] = bwlabeln(rawImage > threshold, 8);

for label=1:num
    [y x] = find(labeled == label);
    nodex(label) = mean(x);
    nodey(label) = mean(y);
end

nodes = [nodex' nodey'];