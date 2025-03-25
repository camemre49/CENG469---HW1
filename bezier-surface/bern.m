function [ res ] = bern( i, n, t )

n_choose_i = factorial(n) / (factorial(i) * factorial(n - i));

res = n_choose_i * (t .^ i) .* ((1 - t) .^ (n - i));


end

