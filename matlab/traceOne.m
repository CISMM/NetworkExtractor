% Traces out a tube until some stopping criteria is met.

function [xs ys xNew yNew thetaNew] = ...
    traceOne(I, x, y, xs, ys, radius, theta, sigma, stepSize, leftBound, rightBound)

xNew = [];
yNew = [];
thetaNew = [];

failTriesLeft = 5;

for step=1:1000
    
    % Take step forward in direction of the first peak.
    x = x + stepSize * cos(theta);
    y = y + stepSize * sin(theta);
    
    % See if we have already traced near this point. If so, return.
    % WARNING: slow N^2 method!
    diffX = x-xs;
    diffY = y-ys;
    skip = sum(diffX.*diffX + diffY.*diffY < 0.5 * stepSize*stepSize);
    if (skip > 0)
        'returning'
        return;
    end
    
    % Check to make sure we haven't left the valid image region.
    if (sum(floor([y x]) - leftBound <= 0) > 0 || ...
        sum(rightBound - floor([y x]) <= 0) > 0)
        break;
    end
    
    % Plot sample on the image.
    figure(1);
    hold on;
    plot(x, y, 'r.');
    hold off;
    
    % Find potential branches at this point, excluding the branch from
    % which we arrived.
    [branchTheta branchResponse weakestResponse] = ...
        branchAngles(I, x, y, theta-0.25*pi, theta+0.25*pi, radius, sigma);
    
    if (length(branchTheta) < 1)
        break;
    end
    
    % Pick the strongest next direction, making sure we don't go backwards.
    [branchTheta indices] = sort(branchTheta);
    theta = branchTheta(end);
    branchResponse(indices) = branchResponse;
    
    % Test for stopping criteria heuristics. Use ratio of branch response
    % to weakest response to determine if there is a preferred direction.
    if (branchResponse(1) / weakestResponse < 1.025)
        failTriesLeft = failTriesLeft - 1;
        if (failTriesLeft == 0)
            break;
        end
    else
        failTriesLeft = 5;
    end
    
    % Check for branches along the way.
    [branchTheta branchResponse] = ...
        branchAngles(I, x, y, theta-0.75*pi, theta-0.25*pi, radius, sigma);
    [bt br] = ...
        branchAngles(I, x, y, theta+0.25*pi, theta+0.75*pi, radius, sigma);
    branchTheta = [branchTheta bt];
    for i=1:length(branchTheta)
%        hold on;
%        bx = x + 5*cos(branchTheta(i));
%        by = y + 5*sin(branchTheta(i));
%        plot([x bx], [y by], 'g-');
       
       % At this point to list of new starting points.
       xNew = [xNew x];
       yNew = [yNew y];
       thetaNew = [thetaNew branchTheta(i)];
    end
    
    xs = [xs x];
    ys = [ys y];
    
end