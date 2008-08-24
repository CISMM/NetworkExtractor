function radialProfile(image, segmentation, px, py, i, j, radius, pctThreshold)

s = size(image);
[x y] = meshgrid(1:s(2), 1:s(1));
divs = 64;
theta = 0:pi/divs:2*pi-(pi/divs);
normx = cos(theta);
normy = sin(theta);
interpx = radius*normx;
interpy = radius*normy;

vals = interp2(x, y, image, interpx+i, interpy+j, 'linear') .* ...
    interp2(x, y, segmentation, interpx+i, interpy+j, 'linear');
vecx = interp2(x, y, px, interpx+i, interpy+j, 'linear');
vecy = interp2(x, y, py, interpx+i, interpy+j, 'linear');
dots = abs((vecx .* normx) + (vecy .* normy));
dots = dots .* vals;

% Find mean of radial profile values
profileMean = mean(vals);
profileStd  = std(vals);
        
% Binarize the profile according to threshold
binaryVals = vals > (profileMean + (pctThreshold * profileStd));
crossings = diff([binaryVals binaryVals(1)]);

span = find(crossings > 0) - find(crossings < 0);
fiberWidth = 3.0 / ((2*pi/divs)*radius);
sum(abs(span) > fiberWidth)

figure(2);
plot(theta, vals);
hold on;
plot([theta(1) theta(end)], [profileMean profileMean] + (pctThreshold * profileStd), 'g-');
hold off;
figure(3);
plot(theta, dots);