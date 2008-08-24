% Computes image where the pixel values come from evaluating a Gaussian at 
% each pixel's distance from the given line.
% 
% ARGUMENTS
%   p1    - First endpoint of line
%   p2    - Second endpoint of line
%   sigma - Standard deviation of Gaussian being evaluated
% 
% OUTPUT
%   output  - Image of filter
%   offsetx - X-offset by which the returned image should be shifted for
%             the points to wind up in the correct position in an image to
%             which the output is being added or appended
%   offsety - Same as x-offset, but in the y-dimension

function [output, offsetx, offsety] = gaussianLine(p1, p2, sigma)

pad = 6*sigma;
xmin = floor(min([p1(1), p2(1)]) - pad);
xmax = ceil(max([p1(1), p2(1)]) + pad);
ymin = floor(min([p1(2), p2(2)]) - pad);
ymax = ceil(max([p1(2), p2(2)]) + pad);
xSize = xmax - xmin + 1;
ySize = ymax - ymin + 1;

% Compute grid points
colOffset = 1 - xmin;
for i=xmin:xmax
    xpts(:,i + colOffset) = i * ones([ySize, 1]);
end

rowOffset = 1 - ymin;
for j=ymin:ymax
    ypts(j + rowOffset,:) = j * ones([1, xSize]);
end

% Compute distance from line
v = p2 - p1;
vmag = norm(v);

% For each pixel, compute projection of vector from pixel to p1 onto vprime
rx = xpts - p1(1);
ry = ypts - p1(2);
dot = (v(1)*rx + v(2)*ry);
t = sign(dot) .* (abs(dot)) / (vmag.^2);

% Get distance to closest point on infinite line
iptx = v(1)*t + p1(1);
ipty = v(2)*t + p1(2);
d = sqrt((xpts - iptx).^2 + (ypts - ipty).^2);

% Get distance from points 1 and 2
dp1 = sqrt((xpts - p1(1)).^2 + (ypts - p1(2)).^2);
dp2 = sqrt((xpts - p2(1)).^2 + (ypts - p2(2)).^2);

% Replace values in distance map where t < 0.0 or t > 1.0
tGreaterThanIndices    = find(t > 1.0);
tLessThanIndices       = find(t < 0.0);
d(tLessThanIndices)    = dp1(tLessThanIndices);
d(tGreaterThanIndices) = dp2(tGreaterThanIndices);

% Use d as parameter in Gaussian function
m = 1 / (sqrt(2*pi)*sigma);
g = m*exp(-d.^2/(2*sigma^2));

output = g;
offsetx = -colOffset+1;
offsety = -rowOffset+1;