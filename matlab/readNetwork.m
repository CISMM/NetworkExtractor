% Function for quickly loading an image of a network. The file name is
% hard-coded here, so you don't need to type it every time you load MATLAB.

function network = readNetwork()

network = imread('network2-1-frame0000.tif');
network = network(10:end,:);
network = network(75:end,185:end);
network = network(:,1:310);

network = double(network);