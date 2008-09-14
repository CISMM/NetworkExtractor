% Optimization functional.
%
% ARGUMENTS
%   I      - Input image.
%   x      - X-position of sample.
%   y      - Y-position of sample.
%   radius - Radius (length) of filter.
%   theta  - Angle of filter.
%   sigma  - Width of line in filter.
%
% OUTPUT
%   Filter response from this set of parameters.

function R = orientedLineFilterResponse(I, x, y, radius, theta, sigma)

ix = floor(x);
iy = floor(y);
xOff = x - ix;
yOff = y - iy;

filter = orientedGaussianLine([xOff yOff], radius, theta, sigma);

hfs = floor(size(filter) / 2);
ys = iy-hfs(1):iy+hfs(1);
xs = ix-hfs(2):ix+hfs(2);

imagePatch = I(ys,xs);

R = sum(sum(filter .* imagePatch));