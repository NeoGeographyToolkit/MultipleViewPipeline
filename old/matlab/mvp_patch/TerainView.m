classdef TerainView < handle
    % status: visible, half-visible, invisible
    properties        
        H               % geographic homography
        w = [192 192]';   % size of terrain
        q               % quaternion elevation
        r               % radial elevation
        s               % scale
    end
    
    properties (Constant)
        invisible = -1e5;
    end % properties (Constant)

    methods
        function tv = TerainView(H,n)
            % Constructor assigns property values
            if nargin > 0,
                tv.H = H;
                q = nan([n n 4]);
                r = nan([n n 1]);
                s = nan([n n 3]);
            end
        end
        
        function [e,d,a] = post(tv,z)
            [e,d,a]=pix2dir(tv.H,z);
        end
        
        function w = size(tv)
            w = tv.w;
        end
        
        function [I,J]=order(tv)
            [I,J]=find(isnan(tv.r) & tv.r ~= TerainView.invisible);
            D = (I-mv.w(2)).^2+(J-mv.w(1)).^2;	% distance from seed
            DIJ = sortrows([D I J]);					% sorted distance
            I = DIJ(:,2); J = DIJ(:,3);					% sorted pixels
        end
        
        function h=disp(tv)
            h=figure; title('DEM'); mesh(tv.r); 
        end
    end % methods
end % classdef

function [e,d,a]=pix2dir(H,z)
% compute spherical direction from orthographic image coordinates.

sz = size(z);
% spherical angles from pixel coordinates in a orthographic image
a = H*[z;ones(1,sz(2))];        % spherical angles (longitude and latitude)
a = a(1:2,:)./[a(3,:);a(3,:)];  % inhomonizing the coordinates (radians)
ca = cos(a);        % cosine of spherical angles
sa = sin(a);        % sine of spherical angles

% direction vector and its Jacobian
e = [ca(1,:).*ca(2,:); sa(1,:).*ca(2,:); sa(2,:)];
if nargout > 1
    % longitudinal and latitudianl Jacobians
    dlon = [-e(2,:); e(1,:); zeros(1,sz(2))];
    dlat = [-ca(1,:).*sa(2,:); -sa(1,:).*sa(2,:); ca(2,:)];
    d = reshape([dlon; dlat],[3,2,sz(2)]);
end
end