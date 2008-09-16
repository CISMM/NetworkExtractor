% Computes a 2D isotropic Gaussian with standard deviation sigma with
% subpixel offset.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%   xOff  - subpixel offset for the Gaussian in x
%   yOff  - subpixel offset for the Gaussian in y
%
% OUTPUT
%   g2    - 2D Gaussian with given standard deviation

function g2 = G2sp(sigma, xOff, yOff)

extent = ceil(6*sigma);
[x y] = meshgrid(-extent:extent, -extent:extent);
x = x - xOff;
y = y - yOff;

g2 = (1.0 / (2.0*pi*sigma^2)) * exp(-(x.^2 + y.^2)/(2*sigma^2));