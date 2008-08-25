% Cleans up a binary image by removing connected components consisting of
% fewer than n pixels.
%
% ARGUMENTS
%   bw    - Binary image containing pixels with value 0 or 1.
%   n     - Minimum number of pixels in a connected component that is
%           preserved by this filter.
%
% OUTPUT
%   clean - Image with small connected components removed.

function clean = removeSmallIslands(bw, n)

clean = bw;
[L, num] = bwlabel(bw);
for label=1:num
    indices = find(L == label);
    if (length(indices) < n)
        clean(indices) = 0;
    end
end
