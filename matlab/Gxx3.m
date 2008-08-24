% Creates 3D image of second partial derivative of isotropic Gaussian in x.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dxx   - 3D image of second partial derivative of isotropic Gaussian in x

function dxx = Gxx3(sigma)

g1 = gaussian(sigma);
gxx2 = Gxx2(sigma);
zDim = size(gxx2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gxx2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dxx(j,i,:) = permute(value * g1, [3,1,2]);
    end
end