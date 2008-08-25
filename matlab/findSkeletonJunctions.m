% Identifies junctions in skeletonized images. Any "on" pixel in a skeleton
% image that has more than two "on" neighbors is considered a junction
% pixel.
%
% ARGUMENTS
%   skel      - Skeleton image to process.
%
% OUTPUT
%   junctions - Image where identified junction pixels have value 1.

function junctions = findSkeletonJunctions(skel)

    junctions = zeros(size(skel));
    
    % Create list of junction templates of cardinality 3.
    t1 = [0 1 0;
          1 1 1;
          0 0 0;];
    t2 = [0 1 0;
          0 1 1;
          1 0 0;];
    t3 = [0 1 0;
          1 1 0;
          0 0 1;];
    t4 = [0 0 1;
          1 1 0;
          0 0 1;];
    t5 = [0 1 0;
          1 1 1;
          0 0 1;];
    t6 = [0 1 0;
          1 1 1;
          1 0 0;];
    t7 = [1 1 0;
          1 1 1;
          1 0 0;];
    t8 = [1 0 0;
          1 1 1;
          1 1 0;];
    t9 = [1 1 1;
          1 1 1;
          1 0 0;];
    t10 = [1 0 0;
           1 1 1;
           1 1 1;];

    [height width] = size(skel);
    for j=2:height-1
        for i=2:width-1
            if skel(j,i) == 0
                continue;
            end
            
            % Exam 8 neighbors of a pixel (includes diagonals).
            patch = skel(j-1:j+1,i-1:i+1);
            
            templates.t1  = t1;
            templates.t2  = t2;
            templates.t3  = t3;
            templates.t4  = t4;
            templates.t5  = t5;
            templates.t6  = t6;
            templates.t7  = t7;
            templates.t8  = t8;
            templates.t9  = t9;
            templates.t10 = t10;
            
            % Template matching loop. Loop over 4 iterations of rotations.
            for t=1:4
                
                tmatch = zeros(8,1);
                tmatch( 1) = sum(sum(patch == templates.t1 )) == 9;
                tmatch( 2) = sum(sum(patch == templates.t2 )) == 9;
                tmatch( 3) = sum(sum(patch == templates.t3 )) == 9;
                tmatch( 4) = sum(sum(patch == templates.t4 )) == 9;
                tmatch( 5) = sum(sum(patch == templates.t5 )) == 9;
                tmatch( 6) = sum(sum(patch == templates.t6 )) == 9;
                tmatch( 7) = sum(sum(patch == templates.t7 )) == 9;
                tmatch( 8) = sum(sum(patch == templates.t8 )) == 9;
                tmatch( 9) = sum(sum(patch == templates.t9 )) == 9;
                tmatch(10) = sum(sum(patch == templates.t10)) == 9;
                
                if (sum(tmatch) > 0)
                    junctions(j,i) = 1;
                end;
                
                templates.t1  = rightRotate(templates.t1 );
                templates.t2  = rightRotate(templates.t2 );
                templates.t3  = rightRotate(templates.t3 );
                templates.t4  = rightRotate(templates.t4 );
                templates.t5  = rightRotate(templates.t5 );
                templates.t6  = rightRotate(templates.t6 );
                templates.t7  = rightRotate(templates.t7 );
                templates.t8  = rightRotate(templates.t8 );
                templates.t9  = rightRotate(templates.t9 );
                templates.t10 = rightRotate(templates.t10);
                
            end
        end
    end
    
end