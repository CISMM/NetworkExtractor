% Creates 3D image of first partial derivative of isotropic Gaussian in x.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dx    - 3D image of first partial derivative of isotropic Gaussian in x

function dx = Gx3(sigma)

g1 = gaussian(sigma);
gx2 = Gx2(sigma);
zDim = size(gx2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gx2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dx(j,i,:) = permute(value * g1, [3,1,2]);
    end
end