% Rotates a matrix to the right 90 degrees.
%
% ARGUMENT
%   m       - The matrix to rotate.
%
% OUTPUT
%   rotated - The rotated matrix.

function rotated = rightRotate(m)
    rotated = zeros([size(m,2), size(m,1)]);
    
    [y x] = size(m);
    for i=1:x
        rotated(i,:) = m(end:-1:1,i);
    end

end