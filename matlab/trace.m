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
branchTheta = branchAngles(I, startX, startY, 0, 2*pi, 1, radius, sigma);

% Optimize starting position.
[startX startY] = optimizePosition(I, startX, startY, branchTheta(1), ...
    stepSize, sigma);

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
xNew = startX*ones(size(branchTheta)); % Queue for new branch points to traverse.
yNew = startY*ones(size(branchTheta));
thetaNew = branchTheta;

% Start traversal of branches
maxI = length(xNew);
i = 1;
while (i <= maxI)
    % Plot starting point.
    hold on;
    plot(xNew(i), yNew(i), 'r.');
    hold off;
    
    x = xNew(i);
    y = yNew(i);
    
    [xt yt tt] = traceOne(I, x, y, xs, ys, radius, ...
        thetaNew(i), sigma, stepSize, leftBound, rightBound);
    xs = [xs xt];
    ys = [ys yt];
    
    [bx by bt] = findBranches(I, xt, yt, tt, radius, sigma);
    bxR = bx + 5*cos(bt);
    byR = by + 5*sin(bt);
    plot(bx, by, 'ys');
    for b=1:length(bxR)
         plot([bx(b) bxR(b)], [by(b) byR(b)], 'b');
    end
     
    % Plot lines connecting branch points.
    %if (sum(size(xt)) > 0)
    %    plot([x bx xt(end)], [y by yt(end)], 'w-');
    %end
    
    xNew = [xNew bx];
    yNew = [yNew by];
    thetaNew = [thetaNew bt];
    
    maxI = length(xNew);
    i = i + 1;
    %pause;
end

%plot(xs, ys, 'r.');
