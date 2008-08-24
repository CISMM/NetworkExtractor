% Generate shaded isosurface of 3D array at given isocontour value.
%
% ARGUMENTS
%   data     - 3D grid data from which isosurface should be taken.
%   isovalue - Value at which isosurface is taken.

function showIsosurface(data, isovalue)

dataSize = size(data);

[x, y, z] = meshgrid(1:dataSize(1), 1:dataSize(2), 1:dataSize(3));
p = patch(isosurface(x,y,z,data,isovalue));
isonormals(x,y,z,data,p);
set(p,'FaceColor','red','EdgeColor','none');
view(3); axis vis3d;
camlight right;
lighting gouraud