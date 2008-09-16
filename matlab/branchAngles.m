% Returns a set of angles corresponding to peak responses of the oriented
% line probe.

function [theta peakResponse weakestResponse] = ...
    branchAngles(I, startX, startY, startTheta, endTheta, cycle, radius, sigma)

angles = startTheta:pi/64:endTheta;
response = zeros(1, length(angles));
for i=1:length(angles)
    theta = angles(i);
    response(i) = orientedLineFilterResponse(I, startX, startY, radius, theta, sigma);
end

weakestResponse = min(response);

% Take first derivative of response function.
dr = diff(response);

if (cycle)
    dr(end+1) = response(1)-response(end);
end

% Identify zero-crossings of first derivative corresponding to peaks.
pos = dr >= 0;
peaks = [];
for i=1:length(dr)-1
    if (pos(i) == 1 && pos(i+1) == 0)
        peaks = [peaks i+1];
    end
end
if (cycle)
    if (pos(end) == 1 && pos(1) == 0)
        peaks(end+1) = length(dr);
    end
end

theta = angles(peaks);
peakResponse = response(peaks);