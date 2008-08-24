% Creates 2D image of first partial derivative of isotropic Gaussian in y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dy    - 2D image of first partial derivative of isotropic Gaussian in y

function dy = Gy2(sigma)

dy = gaussian1stDeriv(sigma)'*gaussian(sigma);
dy = dy./sum(sum(abs(dy)));