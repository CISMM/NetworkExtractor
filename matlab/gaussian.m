% Computes a 1D Gaussian with standard deviation sigma.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%
% OUTPUT
%   G     - 1D array containing Gaussian

function G = gaussian(sigma)

m = 1 / (sqrt(2*pi)*sigma);
x = floor(-6*sigma):ceil(6*sigma);
G = m*exp(-x.^2/(2*sigma^2));
G = G./sum(G);