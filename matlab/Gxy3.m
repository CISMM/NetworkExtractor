% Creates 3D image of first partial derivatives of isotropic Gaussian in x
% and y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dxy   - 3D image of first partial derivatives of isotropic Gaussian in
%           x and y

function dxy = Gxy3(sigma)

g1 = gaussian(sigma);
gxy2 = Gxy2(sigma);
zDim = size(gxy2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gxy2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dxy(j,i,:) = permute(value * g1, [3,1,2]);
    end
end