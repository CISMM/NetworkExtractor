% Computes various shape related aspects of a 2D image, including:
%
% ARGUMENTS
%   image - 2D image of type double
%   sigma - scale at which curvature estimates should be taken
%
% OUTPUT
%   k1         - the first principal curvature directon (convention: k1 > k2)
%   k2         - the second principal curvature direction
%   shape      - Koenderink's shape index
%   curvedness - scale term tied to Koenderink's shape index
%   princx     - x-component of principle direction associated with maximum
%                absolute curvature
%   princy     - y-component of principle direction associated with maximum
%                absolute curvature
    
function [k1, k2, shape, curvedness, princx, princy] = curvatures2(image, sigma)

dxx = conv2(image, Gxx2(sigma), 'valid');
dxy = conv2(image, Gxy2(sigma), 'valid');
dyy = conv2(image, Gyy2(sigma), 'valid');

s          = size(dxx);
k1         = zeros(s);
k2         = zeros(s);
princx     = zeros(s);
princy     = zeros(s);
shape      = zeros(s);
curvedness = zeros(s);

for j=1:s(1)
    for i=1:s(2)
        % Form the Hessian
        Lxx = dxx(j,i);
        Lxy = dxy(j,i);
        Lyy = dyy(j,i);
        H = [Lxx Lxy; Lxy Lyy];
        
        % Eigenanalysis on the Hessian
        [eigVectors eigValues] = eig(H);
        kA = eigValues(1,1);
        kB = eigValues(2,2);
        
        % Convention is that k1 > k2
        if (kA > kB)
            k1(j,i) = kA;
            k2(j,i) = kB;
        else
            k1(j,i) = kB;
            k2(j,i) = kA;
        end
        
        % Organize principal directions so that first is in direction of
        % least curvature magnitude.
        if (abs(kA) < abs(kB))
            princx(j,i) = eigVectors(1,1);
            princy(j,i) = eigVectors(2,1);
        else
            princx(j,i) = eigVectors(1,2);
            princy(j,i) = eigVectors(2,2);
        end
        shape(j,i) = (2/pi) * atan((k2(j,i) + k1(j,i))/(k2(j,i) - k1(j,i)));
        curvedness(j,i) = (2/pi)*log(sqrt(0.5*(kA^2 + kB^2)));
    end
end

% Pad the derived images to match the original image size
padSize = (size(image) - size(k1))/2;
k1 = padarray(k1, padSize);
k2 = padarray(k2, padSize);
shape = padarray(shape, padSize);
curvedness = padarray(curvedness, padSize);
px = padarray(princx, padSize);
py = padarray(princy, padSize);

% Orient the principal directions so that they are all pointing toward the
% bottom half of the image plane
princx = px .* sign(-py);
princy = py .* -sign(-py);