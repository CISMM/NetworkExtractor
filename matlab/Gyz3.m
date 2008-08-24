% Creates 3D image of first partial derivatives of isotropic Gaussian in y
% and z.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dyz   - 3D image of first partial derivatives of isotropic Gaussian in
%           y and z

function dyz = Gyz3(sigma)

g1 = gaussian1stDeriv(sigma);
gy2 = Gy2(sigma);
zDim = size(gy2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gy2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dyz(j,i,:) = permute(value * g1, [3,1,2]);
    end
end