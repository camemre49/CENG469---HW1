function P_rotated = rotateBezier(P, angle_deg, axis)
    % Convert angle to radians
    angle_rad = deg2rad(angle_deg);

    % Define rotation matrices
    Rx = [1, 0, 0; 
          0, cos(angle_rad), -sin(angle_rad); 
          0, sin(angle_rad), cos(angle_rad)];
      
    Ry = [cos(angle_rad), 0, sin(angle_rad); 
          0, 1, 0; 
          -sin(angle_rad), 0, cos(angle_rad)];
      
    Rz = [cos(angle_rad), -sin(angle_rad), 0; 
          sin(angle_rad), cos(angle_rad), 0; 
          0, 0, 1];

    % Select the appropriate rotation matrix
    switch lower(axis)
        case 'x'
            R = Rx;
        case 'y'
            R = Ry;
        case 'z'
            R = Rz;
        otherwise
            error('Invalid axis. Use "x", "y", or "z".');
    end

    % Apply rotation to each Bézier control point set
    P_rotated = cell(size(P));
    for i = 1:numel(P)
        P_matrix = P{i};

        % Ensure P_matrix is Nx3 before reshaping
        if size(P_matrix, 2) ~= 3
            error("Each control point set must have 3 columns (X, Y, Z).");
        end

        % Reshape for matrix multiplication (3×N format)
        P_reshaped = P_matrix'; % Transpose to get 3×N

        % Apply rotation
        P_rotated_reshaped = R * P_reshaped;

        % Transpose back to Nx3 format
        P_rotated{i} = P_rotated_reshaped';
    end
end
