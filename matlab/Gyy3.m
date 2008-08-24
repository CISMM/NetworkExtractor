% Creates 3D image of second partial derivative of isotropic Gaussian in y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dyy   - 3D image of second partial derivative of isotropic Gaussian in
%           y

function dyy = Gyy3(sigma)

g1 = gaussian(sigma);
gyy2 = Gyy2(sigma);
zDim = size(gyy2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gyy2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dyy(j,i,:) = permute(value * g1, [3,1,2]);
    end
end