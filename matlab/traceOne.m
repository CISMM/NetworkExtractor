% Traces out a tube until some stopping criteria is met.

function [xNew yNew thetaNew] = ...
    traceOne(I, x, y, xs, ys, radius, theta, sigma, stepSize, leftBound, rightBound)

xNew = [];
yNew = [];
thetaNew = [];

bunch = 10;
failTriesLeft = 5;

figure(1);
hold on;
for step=1:1000

    % Take step forward in direction of the first peak.
    x = x + stepSize * cos(theta);
    y = y + stepSize * sin(theta);

    % See if we have already traced near this point. If so, stop tracing.
    if (intersectsTube(x, y, xs, ys, stepSize) || ...
        withinValidImageRegion(x, y, leftBound, rightBound))
        return;
    end
    
    % Optimize position along vector perpendicular to the search direction.
    [x y] = optimizePosition(I, x, y, theta, stepSize, sigma);

    % Check that there is more tube to trace.
    [branchTheta branchResponse weakestResponse] = ...
        branchAngles(I, x, y, theta-0.25*pi, theta+0.25*pi, 0, ...
        radius, sigma);
    
    if (length(branchTheta) < 1)
        break;
    end

    % Pick the strongest next direction, making sure we don't go backwards.
    [maxResponse idx] = max(branchResponse);
    theta = branchTheta(idx);
    
    % Test for stopping criteria heuristics. Use ratio of branch response
    % to weakest response to determine if there is a preferred direction.
    if (maxResponse / weakestResponse < 1.025)
        failTriesLeft = failTriesLeft - 1;
        if (failTriesLeft == 0)
            break;
        end
    else
        failTriesLeft = 5;
    end

    % This is necessary to keep the current list of sample points up to
    % date for when we check if we run into another tube that has already
    % been extracted.
    xs = [xs x];
    ys = [ys y];
    
    % Add to list of samples on this tube.
    xNew = [xNew x];
    yNew = [yNew y];
    thetaNew = [thetaNew theta];
    
    % Show sample point on image.
    if (mod(step, bunch) == 0 && step > 1)
        plot(xNew(step-bunch+1:step), yNew(step-bunch+1:step), 'r.');
        drawnow;
    end

end

% Show last sample points on image.
if (step > 1)
    plot(xNew, yNew, 'r.');
    drawnow;
end

end


%  Checks whether the current sample point is too close to an existing
%  sample point in the set of traced tubes.
function collision = intersectsTube(x, y, xs, ys, stepSize)

% WARNING: slow N^2 method!
diffX = x-xs;
diffY = y-ys;
collision = sum(diffX.*diffX + diffY.*diffY < 0.25 * stepSize*stepSize);

end


% Determines whether the sample point is within the 'valid' region in the
% image, that is, the region where it is safe to apply the oriented line
% filter response routine.
function inside = withinValidImageRegion(x, y, leftBound, rightBound)
    inside = sum(floor([y x]) - leftBound <= 0) > 0 || ...
        sum(rightBound - floor([y x]) <= 0) > 0;

end