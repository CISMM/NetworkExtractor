% Creates 3D image of first partial derivative of isotropic Gaussian in y.
%
% ARGUMENTS
%   sigma - Standard deviation of Gaussian
%
% OUTPUT
%   dy    - 3D image of first partial derivative of isotropic Gaussian in y

function dy = Gy3(sigma)

g1 = gaussian(sigma);
gy2 = Gy2(sigma);
zDim = size(gy2);

% Now multiply each depth "column" by Gaussian
for j=1:zDim(1)
    for i=1:zDim(2)
        % Get constant value through depth column
        value = gy2(j,i);
        
        % Do multiplication and permute to store in new 3D array
        dy(j,i,:) = permute(value * g1, [3,1,2]);
    end
end