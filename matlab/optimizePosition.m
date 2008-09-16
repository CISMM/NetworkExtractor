function [optX optY] = optimizePosition(I, xHat, yHat, theta, stepSize, sigma)

d = [cos(theta) sin(theta)];
dPerp = [d(2); -d(1)];

pos = [xHat; yHat];

sp = pos - floor(pos);
steps = -stepSize:0.05:stepSize;
response = zeros(length(steps),3);

for i=1:length(steps)
    t = steps(i);
    offset = t*dPerp + sp;
    filter = G2sp(sigma, offset(1), offset(2));
    hfd = floor(size(filter,1) / 2);
    iPos = floor(pos);
    
    patch = I(iPos(2)-hfd:iPos(2)+hfd, iPos(1)-hfd:iPos(1)+hfd);
    response(i,1) = sum(sum(patch .* filter));
    response(i,2) = iPos(1) + offset(1);
    response(i,3) = iPos(2) + offset(2);
end

[m, idx] = max(response(:,1));

optX = response(idx,2);
optY = response(idx,3);