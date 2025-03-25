function bezierSurface(P_list, resolution, showPointsOf) 
    % Color map for scatter points
    colors = ['r', 'g', 'b', 'm'];

    figure;
    hold on;

    for k = 1:length(P_list)
        P = P_list{k}
        s = linspace(0, 1, resolution)
        t = linspace(0, 1, resolution)
        Qx = zeros(resolution, resolution);
        Qy = zeros(resolution, resolution);
        Qz = zeros(resolution, resolution);

        % --- Plot Control Points ---
        control_x = zeros(4,4);
        control_y = zeros(4,4);
        control_z = zeros(4,4);
        
        for i = 1:4
            for j = 1:4
                control_x(i,j) = P{i,j}(1);
                control_y(i,j) = P{i,j}(2);
                control_z(i,j) = P{i,j}(3);
            end
        end
    
        
        if k == showPointsOf || showPointsOf == -1
            % Plot control net (blue lines)
            for i = 1:4
                plot3(control_x(i,:), control_y(i,:), control_z(i,:), 'b-', 'LineWidth', 1.5);
                plot3(control_x(:,i), control_y(:,i), control_z(:,i), 'b-', 'LineWidth', 1.5);
            end
        endif

        for i = 0:3
            for j = 0:3
                Qx = Qx + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(1);
                Qy = Qy + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(2);
                Qz = Qz + bern(i, 3, s)' * bern(j, 3, t) * P{i+1, j+1}(3);

                if k == showPointsOf || showPointsOf == -1
                    % Plot each control point with a specific color
                    scatter3(P{i+1, j+1}(1), P{i+1, j+1}(2), P{i+1, j+1}(3), ...
                            100, colors(i+1), 'filled', 's'); % Use color based on i value
                endif
            end
        end

        surf(Qx, Qy, Qz);
    end

    % Add axis labels
    xlabel('X-axis');
    ylabel('Y-axis');
    zlabel('Z-axis');
    % Improve visualization
    axis equal; grid on;

    hold off;
end
