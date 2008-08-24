% Creates 2D image of second partial derivative of isotropic Gaussian in x.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dxx   - 2D image of second partial derivative of isotropic Gaussian in x

function dxx = Gxx2(sigma);

dxx = gaussian(sigma)'*gaussian2ndDeriv(sigma);
dxx = dxx./sum(sum(abs(dxx)));