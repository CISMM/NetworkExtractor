% Computes a 3D isotropic Gaussian with standard deviation sigma.
%
% ARGUMENTS
%   sigma - standard deviation of Gaussian
%
% OUTPUT
%   g3    - 2D Gaussian with given standard deviation

function g3 = G3(sigma)

g1 = gaussian(sigma);
g2 = g1'*g1;
zDim = size(g2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = g2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        g3(j,i,:) = permute(value * g1, [3,1,2]);
    end
end