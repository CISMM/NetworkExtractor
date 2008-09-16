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
for i=1:length(xs)
    
    x = xs(i);
    y = ys(i);
    theta = thetas(i);
    
    [a1 r1 w1] = ...
        branchAngles(I, x, y, theta-0.75*pi, theta-0.25*pi, 0, radius, sigma);
    [a2 r2 w2] = ...
        branchAngles(I, x, y, theta+0.25*pi, theta+0.75*pi, 0, radius, sigma);
    angles = [a1 a2];
    responses = [r1 r2];
    weakestResponse = min(w1, w2);
    
    if (length(angles) == 0)
        continue;
    end
    
    % Find maximum response.
    [maxResponse idx] = max(responses);
    maxAngle = angles(idx);
    
    if (weakestResponse / maxResponse > (1.0 / 1.025))
        continue;
    end
    
    % Keep top angle from the search.
    branchResponse(i) = maxResponse;
    branchAngle(i)    = maxAngle;
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