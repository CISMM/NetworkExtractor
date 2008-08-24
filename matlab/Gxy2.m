% Creates 2D image of first partial derivatives of isotropic Gaussian in x
% and y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dxy   - 2D image of first partial derivatives of isotropic Gaussian in
%           x and y

function dxy = Gxy2(sigma);

dxy = gaussian1stDeriv(sigma)'*gaussian1stDeriv(sigma);
dxy = -dxy./sum(sum(abs(dxy)));