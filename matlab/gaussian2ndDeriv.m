% Computes the second derivative of a 1D Gaussian with standard deviation 
% sigma.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%
% OUTPUT
%   GPP   - 2D array containing Gaussian

function GPP = gaussian2ndDeriv(sigma)

m = 1/(sqrt(2*pi)*sigma^3);
x = floor(-6*sigma):ceil(6*sigma);
GPP = m*((x.^2/sigma^2) - 1).*exp(-x.^2/(2*sigma^2));
GPP = GPP/sum(abs(GPP));