% Generates a binary mask of an image by marking whether a pixel has the
% maximum value in a neighborhood. If two or more pixels tie for maximum,
% they are all marked.
%
% ARGUMENTS
%   image  - Input image.
%   nhsize - Neighborhood size.
%
% OUTPUT
%   output - Image pixels whose values are the highest in their
%            neighborhood.

function output = maxInNeighborhood(image, nhsize)

s = size(image);

% Initialize output
output = zeros(size(image));

% Generate mask
mask = ones(2*nhsize + 1);
kernelSize = 2*nhsize + 1;

for j=1:s(1)-kernelSize
    for i=1:s(2)-kernelSize
        neighborhood = image(j:j+kernelSize,i:i+kernelSize);
        m = max(max(neighborhood));
        [maxIndicesY maxIndicesX] = find(neighborhood == m);
        for k=1:length(maxIndicesY);
            output(j+maxIndicesY(k)-1, i+maxIndicesX(k)-1) = ...
                image(j+maxIndicesY(k)-1, i+maxIndicesX(k)-1);
            output(j+maxIndicesY(k)-1, i+maxIndicesX(k)-1) = 1;
        end
    end
end