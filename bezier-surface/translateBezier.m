function P_translated = translateBezier(P, tx, ty, tz)
    P_translated = cell(size(P)); % Initialize the translated cell array
    for i = 1:numel(P)
        P_translated{i} = P{i} + [tx, ty, tz]; % Apply translation to each matrix
    end
end
