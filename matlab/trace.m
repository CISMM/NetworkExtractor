% Traces a mesh of tubes in 2D images.

function trace(I)

radius = 5;
sigma = 1.0;
stepSize = 1;

% Show image
figure(1);
imagescale(I);
drawnow;

% Ask for starting point on the input mesh.
'Click on a starting point on the mesh...'
[startX,startY] = ginput(1);
offX = startX - floor(startX);
offY = startY - floor(startY);
startX = floor(startX);
startY = floor(startY);

% Scan around for a starting direction
branchTheta = branchAngles(I, startX, startY, 0, 2*pi, radius, sigma);

% Plot the lines of angles with best response on the image.
figure(1);
hold on;
plot(startX, startY, 'ys');

for i=1:length(branchTheta)
    lx = [startX 5.0*cos(branchTheta(i)) + startX];
    ly = [startY 5.0*sin(branchTheta(i)) + startY];
    plot(lx, ly, 'g');
end

hold off;

% Find region of the image where it is legal to track. This is the interior
% region of the image eroded by half the width of the oriented line filter.
f = orientedGaussianLine([0 0], radius, 0, sigma);
leftBound  = floor(size(f)/2);
rightBound = size(I) - floor(size(f)/2);

% Start tracing along the mesh.
xs = [];
ys = [];
xNew = []; % Queue for new branch points to traverse.
yNew = [];
thetaNew = [];
for i=1:length(branchTheta)
    % Move one step away from the starting point so that we don't get
    % caught going down the same branch twice.
    x = startX + stepSize*cos(branchTheta(i));
    y = startY + stepSize*sin(branchTheta(i));
    [xs ys xn yn tn] = traceOne(I, x, y, xs, ys, radius, ...
        branchTheta(i), sigma, stepSize, leftBound, rightBound);
    xNew = [xNew xn];
    yNew = [yNew yn];
    thetaNew = [thetaNew tn];
end

% Start traversal of identified branches
maxI = length(xNew);
i = 1;
while (i < maxI)
    % Move one step away from the starting point so that we don't get
    % caught going down the same branch twice.
    x = startX + stepSize*cos(thetaNew(i));
    y = startY + stepSize*sin(thetaNew(i));
    
    [xs ys xn yn tn] = traceOne(I, xNew(i), yNew(i), xs, ys, radius, ...
        thetaNew(i), sigma, stepSize, leftBound, rightBound);
    xNew = [xNew xn];
    yNew = [yNew yn];
    thetaNew = [thetaNew tn];
    
    maxI = length(xNew);
    i = i + 1;
end
