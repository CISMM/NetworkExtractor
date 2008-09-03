% Verifies intersections in images of network structures and optimizes
% their positions. Returns the verified junction positions and the
% cardinality of the junction.
%
% ARGUMENTS
%   image         - Image of network.
%   junctions     - Initial sets of candidate junction points.
%   neighborhood  - Neighborhood that should be searched for the optimal
%                   junction position.
%   darkOnLight   - Boolean indicating whether network image consists of
%                  dark-on-light segments.

%
% OUTPUT
%   positions     - The positions of the verified junctions.
%   cardinalities - The cardinality of the verified junctions.

function [positions, cardinalities] = optimizeJunctions(image, junctions, neighborhood, darkOnLight)
    
    % Iterate over the candidate junction points.
    for ptIdx = 1:size(junctions,1);
        [ix iy] = junctions(ptIdx,:) 
    
        % Optimize over scale
        for sigma=2.0:1.0:2.0
       
            % Optimize over position (translations only).
            for iy-neighborhood:1:iy+neighborhood
                for ix-neighborhood:1:ix+neighborhood
            
                % Search for an additional "arm" of the junction.
                
    
                end
            end
        end
    end
    
    positions = junctions;
    cardinalities = [];
end