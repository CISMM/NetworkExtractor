% Plots a regular sub-sampling of vectors in a quiver plot. Samples every
% nth vector from (px, py) in each dimension.
%
% ARGUMENTS
%   px - List of x-components from points.
%   py - List of y-components from points.
%   n  - Stride of point sampling.
%
% OUTPUT
%   Displays quiver plot in currently-active figure.

function sparseQuiver(px, py, n)

pxSize = size(px);
[xgrid ygrid] = meshgrid(1:n:pxSize(2), 1:n:pxSize(1));
quiver(xgrid, ygrid, px(1:n:end,1:n:end), py(1:n:end,1:n:end));