% Creates a filter taking 2nd derivative in arbitrary direction v defined 
% by theta, the angle of v from positive x-axis.
%
% ARGUMENTS
%   sigma - Scale of the filter
%   theta - Angle (in radians) of direction v along which 2nd derivative
%           should be computed
%
% OUTPUT
%   dvv   - Image containing the oriented 2nd derivative filter. Lines 
%           oriented at theta + pi/2 give maximal response.

function dvv = Gvv2(sigma, theta)

coords = floor(-6*sigma):ceil(6*sigma);
[x y] = meshgrid(coords, coords);

% Rotate coordinates about origin.
xp = cos(theta)*x - sin(theta)*y;
yp = sin(theta)*x + cos(theta)*y;

m = 1/(sqrt(2*pi)*sigma^3);
dvv = -m*((xp.^2/sigma^2) - 1).*exp(-xp.^2/(2*sigma^2));

m = 1 / (sqrt(2*pi)*sigma);
G = m*exp(-yp.^2/(2*sigma^2));

dvv = dvv .* G;
dvv = dvv ./ sum(sum(dvv));