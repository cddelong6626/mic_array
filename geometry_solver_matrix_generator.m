N_MICS = 6;
RADIUS_CM = 4;


RADIUS_M = RADIUS_CM/100;
positions = [];
for i = 1:N_MICS
    theta = (i-1) * 360/N_MICS;
    positions = [positions; RADIUS_M*cos(theta) RADIUS_M*sin(theta)];
end


A = [];
for i = 1:N_MICS
    for j = i+1:N_MICS
        A = [A; positions(i, :) - positions(j, :)];
    end
end

M = pinv(A' * A) * A';


fprintf('const float M[2][15] = {\n');
for r = 1:size(M,1)
    fprintf('    {');
    for c = 1:size(M,2)
        if c < size(M,2)
            fprintf('%.9ff, ', M(r,c));
        else
            fprintf('%.9ff', M(r,c));
        end
    end
    if r < size(M,1)
        fprintf('},\n');
    else
        fprintf('}\n');
    end
end
fprintf('};\n');