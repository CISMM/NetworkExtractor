% Creates 2D image of first partial derivative of isotropic Gaussian in x.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dx    - 2D image of first partial derivative of isotropic Gaussian in x

function dx = Gx2(sigma);

dx = gaussian(sigma)'*gaussian1stDeriv(sigma);
dx = dx./sum(sum(abs(dx)));