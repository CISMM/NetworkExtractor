function output = laplacian2(sigma)

output = Gxx2(sigma) + Gyy2(sigma);
output = output / sum(sum(abs(output)));