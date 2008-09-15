% Examine the trace points from a tube to see whether branches have been
% encountered.

function [bx by bt] = findBranches(I, xs, ys, thetas, radius, sigma)

bx = [];
by = [];
bt = [];

branchAngle    = zeros(1, length(xs));
branchResponse = zeros(1, length(xs));

% Since we are starting at a branch point and possible ending at a point
% on the network, we shouldn't identify branches near the start or end of
% the tube.
for i=7:length(xs)-6
    
    x = xs(i);
    y = ys(i);
    theta = thetas(i);
    
    [a1 r1] = ...
        branchAngles(I, x, y, theta-0.9*pi, theta-0.1*pi, radius, sigma);
    [a2 r2] = ...
        branchAngles(I, x, y, theta+0.1*pi, theta+0.9*pi, radius, sigma);
    angles = [a1 a2];
    responses = [r1 r2];
    
    if (length(angles) == 0)
        continue;
    end
    
    [angles idx] = sort(angles);
    responses = responses(idx);
    
    % Keep top angle from the search.
    branchAngle(i)    = angles(end);
    branchResponse(i) = responses(end);
end

% Now search through the branch responses and identify local peaks.

% Take first derivative of response function.
dr = diff(branchResponse);

pos = dr >= 0;
peaks = [];
for i=1:length(dr)-1
    if (pos(i) == 1 && pos(i+1) == 0)
        peaks = [peaks i+1];
    end
end

bx = xs(peaks);
by = ys(peaks);
bt = branchAngle(peaks);