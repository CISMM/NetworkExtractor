% Generates a series of network test cases for tube network tracing
% algorithms.

points = [(20:150)' 80+40*(sin((20:150)/20))']

lines = [(1:size(points,1)-1)' (2:size(points,1))'];

S = drawNetwork(200, 1.5, points, lines);

points = [
    20 100;
    150 100;
    150 150;
    150 50;
    ];

lines = [
    1 2;
    2 3;
    2 4;
    ];

T = drawNetwork(200, 1.5, points, lines);

points = [
    20 20;
    20 180;
    180 180;
    180 20;
    ];

lines = [
    1 2;
    2 3;
    3 4;
    4 1;
    ];

BOX = drawNetwork(200, 1.5, points, lines);

points = [
    20 100;
    180 100;
    160 120;
    160 80;
    ];

lines = [
    1 2;
    2 3;
    2 4;
    ];

ARROWHEAD = drawNetwork(200, 1.5, points, lines);

points = [
    20 100;
    160 100;
    180 120;
    180 80;
    ];

lines = [
    1 2;
    2 3;
    2 4;
    ];

ARROWTAIL = drawNetwork(200, 1.5, points, lines);

points = [
    20 100;
    180 100;
    100 20;
    100 180;
    ];

lines = [
    1 2;
    3 4;
    ];

CROSS = drawNetwork(200, 1.5, points, lines);