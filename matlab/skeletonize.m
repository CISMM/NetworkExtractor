% Configure settings for skeletonization.
sigma = 2.0;
k2min = 100;
frames = 0;

% Loop over all frames in the video
for frame=0:frames

    % Load up image
    filename = sprintf('network2-1-frame%04d.tif', frame);
    data = double(imread(filename));
    f = 1;
    %figure(f);
    %f = f + 1;
    %imagescale(data);

    % Compute curvatures of image.
    [k1 k2] = curvatures2(data, sigma);

    % Negate so that we are dealing with positive numbers (which I think
    % are nicer).
    k2 = -k2;

    % Mask the image.
    mask = zeros(size(k2));
    mask(k2 > k2min) = 1;

    % Create the skeleton.
    skel = bwmorph(mask, 'skel', Inf);

    % Clean up the skeleton.
    skel = double(bwmorph(skel, 'spur'));
    skel = removeSmallIslands(skel,8);

    % Display the skeleton image.
    figure(f);
    f = f + 1;
    imagescale(skel);
    
    junctions = findSkeletonJunctions(skel);
    
    figure(f);
    f = f + 1;
    imagescale(junctions);
    
    jx = [];
    jy = [];
    [L, num] = bwlabel(junctions);
    for label=1:num
        [ly lx] = find(L == label);
        lx = mean(lx);
        ly = mean(ly);
        jx(end+1) = lx;
        jy(end+1) = ly;
    end
        
    % Show candidate junctions superimposed over original image.
    figure(f);
    f = f + 1;
    imagescale(data);
    hold on;
    plot(jx, jy, 'gs');

    % Verify junctions.
    [positions cardinalities] = optimizeJunctions(data, [jx' jy'], 4, false);
    
end