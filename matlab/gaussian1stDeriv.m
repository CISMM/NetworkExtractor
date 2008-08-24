% Computes the first derivative of a 1D Gaussian with standard deviation 
% sigma.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%
% OUTPUT
%   GP    - 1D array containing Gaussian

function GP = gaussian1stDeriv(sigma)

m = 1/(sqrt(2*pi)*sigma^3);
x = floor(-6*sigma):ceil(6*sigma);
GP = m*-x.*exp(-x.^2/(2*sigma^2));
GP = GP./sum(abs(GP));