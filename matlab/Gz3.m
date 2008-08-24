% Creates 3D image of first partial derivative of isotropic Gaussian in z.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dz    - 3D image of first partial derivative of isotropic Gaussian in
%           z

function dz = Gz3(sigma)

gd1 = gaussian1stDeriv(sigma);
g2 = G2(sigma);
zDim = size(g2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = g2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dz(j,i,:) = permute(value * gd1, [3,1,2]);
    end
end