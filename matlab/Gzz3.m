% Creates 3D image of second partial derivative of isotropic Gaussian in z.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dyy   - 3D image of second partial derivative of isotropic Gaussian in
%           z

function dzz = Gzz3(sigma)

gd2 = gaussian2ndDeriv(sigma);
g2 = G2(sigma);
zDim = size(g2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = g2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dzz(j,i,:) = permute(value * gd2, [3,1,2]);
    end
end