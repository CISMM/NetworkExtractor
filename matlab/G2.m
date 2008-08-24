% Computes a 2D isotropic Gaussian with standard deviation sigma.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%
% OUTPUT
%   g2    - 2D Gaussian with given standard deviation

function g2 = G2(sigma)

g1 = gaussian(sigma);
s = size(g1);

for i=1:s(2)
    v = g1(i);
    g2(:,i) = v*g1';
end