% Computes the gradient magnitude of a 2D image.
%
% ARGUMENTS
%   image - The image whose gradient magnitude should be calculated
%   sigma - The scale at which the gradient magnitude should be calculated
%
% OUTPUT
%   gradImage - The gradient magnitude image

function gradImage = gradientMagnitude(image, sigma);

dxImage = conv2(image, Gx2(sigma), 'valid');
dyImage = conv2(image, Gy2(sigma), 'valid');

gradImage = sqrt(dxImage.^2 + dyImage.^2);
padSize = (size(image) - size(gradImage)) / 2;

gradImage = padarray(gradImage, padSize);