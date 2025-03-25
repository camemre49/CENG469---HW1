s = linspace(0, 1, 10);
t = linspace(0, 1, 10);

P1 = [0  0 0];
P2 = [0 -1 0];
P3 = [0 -2 0];
P4 = [0 -3 0];

P5 = [3 0  1];
P6 = [3 -1 2];
P7 = [3 -2 2];
P8 = [3 -3 1];

P9 =  [6  0 -1];
P10 = [6 -1 -2];
P11 = [6 -2 -2];
P12 = [6 -3 -1];

P13 = [8 0  0];
P14 = [8 -1 0];
P15 = [8 -2 0];
P16 = [8 -3 0];

P = {P1  P2  P3  P4;
     P5  P6  P7  P8;
     P9  P10 P11 P12;
     P13 P14 P15 P16};
 

Qx = zeros(10, 10);
Qy = zeros(10, 10);
Qz = zeros(10, 10);

for i = 0:3
    for j = 0:3
        Qx = Qx + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(1);
        Qy = Qy + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(2);
        Qz = Qz + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(3);
    end
end

surf(Qx, Qy, Qz);

