% Creates 3D image of first partial derivatives of isotropic Gaussian in x
% and z.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dxz   - 3D image of first partial derivatives of isotropic Gaussian in
%           x and z

function dxz = Gxz3(sigma)

g1 = gaussian1stDeriv(sigma);
gx2 = Gx2(sigma);
zDim = size(gx2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gx2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dxz(j,i,:) = permute(value * g1, [3,1,2]);
    end
end