% Creates 2D image of second partial derivative of isotropic Gaussian in y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dyy   - 2D image of second partial derivative of isotropic Gaussian in
%           y

function dyy = Gyy2(sigma)

dyy = Gxx2(sigma)';