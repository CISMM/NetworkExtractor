% Gets a kernel for oriented line filtering
function f = lineFilterKernel(length, theta, sigma)

theta = -theta; % Accounts for image coordinate system in MATLAB
xx = (length / 2.0) * cos(theta);
yy = (length / 2.0) * sin(theta);

f = gaussian2ndDerivLine([-xx -yy], [xx yy], sigma);

% Set the size of the kernel to be a function of length and sigma, not
% theta so that kernels of any value of theta are the same size.
maxDim = size(gaussian2ndDerivLine([-length/2.0 -length/2.0], [length/2.0 length/2.0], sigma));
padSize = floor((maxDim - size(f))/2);

f = padarray(f, padSize);