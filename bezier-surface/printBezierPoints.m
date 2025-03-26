function printBezierPoints(P)
    printf("std::vector<std::vector<std::vector<double>>> P = {\n");

    for k = 1:length(P)
        printf("    {\n"); % Open a new Bezier surface set
        for i = 1:4
            printf("        {");
            for j = 1:4
                point = P{k}{i, j}; % Extract the point
                printf("{%.6f, %.6f, %.6f}", point(1), point(2), point(3));
                if j < 4
                    printf(", ");
                end
            end
            printf("}");
            if i < 4
                printf(",\n");
            else
                printf("\n");
            end
        end
        printf("    }");
        if k < length(P)
            printf(",\n");
        else
            printf("\n");
        end
    end

    printf("};\n");
end
