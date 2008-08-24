% Junction finding method that looks for pixels with curvedness greater 
% than 4 and shape index 0.1 of -0.5, the shape index of a light-on-dark 
% cylinder. Works terribly.
%
% ARGUMENTS
%   shape      - Image containing shape indices of pixels.
%   curvedness - Image containing curvedness measure of pixels.
%
% OUTPUT
%   x          - X-components of junction points.
%   y          - Y-components of junction points.

function [x y] = shapePlusCurvedness(shape, curvedness)

curvednessMask = zeros(size(curvedness));
curvednessMask(curvedness > 4) = 1;
shapeMask = zeros(size(shape));
shapeMask(abs(shape + 0.5) < 0.2) = 1;

junctionMask = curvednessMask .* shapeMask;

[y x] = find(junctionMask == 1);