% Computes an image of a Gaussian line tilted by an angle about the axis normal 
% to the image plane. The anchor point of the line is always centered 
% within the center pixel of the generated image. 
%
% ARGUMENTS
%   offset - Subpixel offset of the anchor point within the center pixel of
%            the generated image.
%   radius - Radius of line.
%   theta  - Angle of line orientation (in radians);
%   sigma  - Standard deviation of generated line.
%
% OUTPUT
%   result - Square image of the oriented line. Always has odd dimensions.

function result = orientedGaussianLine(offset, radius, theta, sigma)
    dim = ceil(6*sigma) + 2*radius;
    if (mod(dim,2) == 0)
        dim = dim + 1;
    end
    halfDim = floor(dim / 2);
    xpts   = zeros(dim);
    ypts   = zeros(dim);

    % Compute coordinates of grid points in the image.
    for i=1:dim
        xpts(:,i) = (-halfDim + i - offset(1) - 1) * ones([dim, 1]);
    end
    
    for j=1:dim
        ypts(j,:) = (-halfDim + j - offset(2) - 1) * ones([1, dim]);
    end
    
    % Calculate end point.
    p2 = radius * [cos(theta) sin(theta)];
    
    % Compute distance of each pixel coordinate position from the line by
    % projecting the pixel coordinate onto the line.
    v = p2; % Implicit. v = p2 - p1 where p1 is the origin.
    vmag = norm(v);
    dot = (v(1)*xpts + v(2)*ypts);
    t = sign(dot) .* (abs(dot)) / (vmag.^2);
    
    % Get distance to closest point on infinite line.
    iptx = v(1)*t;
    ipty = v(2)*t;
    d = sqrt((xpts - iptx).^2 + (ypts - ipty).^2);
    
    % Get distance from points 1 and 2
    dp1 = sqrt(xpts.^2 + ypts.^2);
    dp2 = sqrt((xpts - p2(1)).^2 + (ypts - p2(2)).^2);
    
    % Replace values in distance map where t < 0.0 or t > 1.0
    tGreaterThanIndices    = find(t > 1.0);
    tLessThanIndices       = find(t < 0.0);
    d(tLessThanIndices)    = dp1(tLessThanIndices);
    d(tGreaterThanIndices) = dp2(tGreaterThanIndices);
    
    % Use d as parameter in Gaussian function
    m = 1 / (sqrt(2*pi)*sigma);
    result = m*exp(-d.^2/(2*sigma^2));
end