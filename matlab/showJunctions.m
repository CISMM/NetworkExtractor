function showJunctions(image, sigma, tolerance, pctMax)

[rawImage nodes] = junctions(image, sigma, tolerance, pctMax);
imagescale(image, 0.8);
hold on;
plot(nodes(:,1), nodes(:,2), 'sg');