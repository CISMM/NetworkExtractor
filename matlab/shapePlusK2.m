% Junction finding method that looks for pixels with shape index greater 
% than a minimum threshold and shape index quite different from the shape 
% index of a cylinder. Works terribly.
%
% ARGUMENTS
%   shape - Image containing shape indices.
%   k2    - Image containing second principal curvature (convention: k2 <
%           k1)
%
% OUTPUT
%   x          - X-components of junction points.
%   y          - Y-components of junction points.

function [x y] = shapePlusK2(shape, k2)

k2Mask = zeros(size(k2));
k2Mask(k2 > 200) = 1;
shapeMinMask = zeros(size(shape));
shapeMinMask(shape < 0.1) = 1;
shapeMaxMask = zeros(size(shape));
shapeMaxMask(shape > 0.9) = 1;

shapeMask = shapeMinMask + shapeMaxMask;
shapeMask(shapeMask > 0) = 1;

junctionMask = k2Mask .* shapeMask;

[y x] = find(junctionMask == 1);