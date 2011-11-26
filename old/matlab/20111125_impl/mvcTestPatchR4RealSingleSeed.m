function mvcPatchR4RealSingleSeed(strOut)
clc
close all
clear all

global MV;
if nargin < 1 | isempty(strOut)
    disp('mvcPatchR4RealSingleSeed(output_file)');
    strOut = '../../real_scene/MVP110902.mat';
end
[pathstr, name, ext] = fileparts(strOut);
if isempty(ext), strOut = [strOut '.mat']; end
if exist(strOut,'file'), 
    load(strOut);
    MV=Views;
    MV.image=imreal(MV);
else
	% initialize the seed by exhaustive search with large window
    MV=mvreal();
    MV.strPath = pathstr;
    MV.strOut = strOut;
    MV.DEM0 = single(nan(1800,1800));
    MV.DEM = single(nan(1800,1800));
    MV.seed = [900 900]'; MV.center = MV.seed;
    MV.DEM0(MV.center(2),MV.center(1))=-5000;
    MV.radiusPatch=[64 12]'; MV.rangeLarge=[-5000 5000]; 
    initMV(); initMVcenter();
    MV.DEM(MV.center(2),MV.center(1)) = InitializeSeed(MV);
    saveDEM(MV);
end

itr = 600;				% printing period of iteration
MV.invisible = -1e5;	% invisible flag
[I,J]=find(isnan(MV.DEM) & MV.DEM ~= MV.invisible);
D = (I-MV.seed(2)).^2+(J-MV.seed(1)).^2;	% distance from seed
DIJ = sortrows([D I J]);					% sorted distance
I = DIJ(:,2); J = DIJ(:,3);					% sorted pixels

MV.radiusPatch=[16 4]'; MV.rangeLarge=[-1000 1000]; initMV();
before = now;
for i=1:length(I)
    MV.center=[J(i); I(i)];
    MV.DEM0(I(i),J(i)) = InitialHeight(MV,I(i),J(i));
    initMVcenter();

	% check the visibility of all views
    visible=mpvisible(MV);
    if visible == false
        MV.DEM(I(i),J(i)) = MV.invisible;
        fprintf('x')
        if rem(i,itr) == 0
            fprintf('\n (%d,%d)',I(i),J(i));
        end
        continue
    end

	% main optimization
    [r,f,exitflag,output] = fminbnd(@(r)mpGausOptQ(r,MV),MV.lb,MV.ub,MV.opts.R);
    fprintf('.');

    MV.DEM(I(i),J(i)) = r-MV.radiusMoon;

	% print and save the result
    if rem(i,itr) == 0
        saveDEM(MV);
        drawPatches(MV,r);
        before = displayComputingTime(before);
    end
end
saveDEM(MV);
end

% initialize height by weighted average in the neighborhood
function h = InitialHeight(Views,i,j)
    sz = size(Views.DEM);
    I = i+[-Views.radiusPatch:Views.radiusPatch];
    Idx = find(I > 0 & I <= sz(1));
    Ic = Views.flt.c0(Idx);
    I = I(Idx);
    
    J = j+[-Views.radiusPatch:Views.radiusPatch];
    Jdx = find(J > 0 & J <= sz(2));
    Jc = Views.flt.c0(Jdx);
    J = J(Jdx);
    
    W = Ic*Jc';
    H = Views.DEM(I,J);
    B = find(~(isnan(H) | H == Views.invisible));
    h = H(B(:))'*W(B(:))/sum(W(B(:)));
end

% display computing time
function after = displayComputingTime(before);
after = now;
str = sprintf('time: %s = %s - %s \n',(after-before)*24*3600,datestr(after),datestr(before));
disp(str);
end

% initialize height at the seed point with large windows
function h=InitializeSeed(MV)
[r,f,exitflag,output] = fminbnd(@(r)mpGausOptQ(r,MV),MV.lb,MV.ub,MV.opts.R);

MV.lb=r+MV.rangeLarge(1);
MV.ub=r+MV.rangeLarge(2);

drawPatches(MV,r)
drawProfile(MV,f,r)
h=r-MV.radiusMoon;
end

% save DEM
function saveDEM(Views)
    Views.DEM;
    i = Views.center(2); j = Views.center(1);
    t = double(Views.DEM0(i,j));
    str = sprintf('\n (%d,%d) Error: %f = %f - (%f)',i,j,Views.DEM(i,j)-t,Views.DEM(i,j),t);
    disp(str);

    Views.image=[];
    save(Views.strOut, 'Views');
end

% draw Patches
function drawPatches(MV,r,q)
if nargin > 1, MV.r = r; end
if nargin > 2, MV.q = q; end
MV.height = MV.r-MV.radiusMoon;

MV.Ortho=mpbackproj(MV);

% rotation matrix
% [Ortho, R]=mpbackproj(MV);
% R
% MV.e'*R(:,3)

for k=1:numel(MV.image)
    figure(1), subplot(2,2,k), imshow(MV.Ortho(:,:,k));
    caption = sprintf('Original Patch %d', k);
    title(caption, 'FontSize', 14);

    figure(2), subplot(2,2,k), histeq(MV.Ortho(:,:,k));
    caption = sprintf('Histogram Equalized %d', k);
    title(caption, 'FontSize', 14);
end
figure(1), subplot(2,2,k), imshow(MV.Ortho(:,:,k));
end

function f=mpGausOptQ(r,Patch)
Patch.r=r;
Patch.Ortho=mpbackproj(Patch);
Patch.Is = conv3(Patch.flt.s0, Patch.flt.s0, Patch.Ortho, 'same');
f = mpreflectance(Patch);
end

% optimal liner reflectance coefficients
function [f,a,b] = mpreflectance(Patch)

% compute Eigen matrix D for a
A = scatter(Patch.Is,Patch.Is,Patch.I,Patch.J,Patch.flt.c0,Patch.flt.c0,Patch.nc,Patch.n,Patch.w(1));
c = wnd3(Patch.flt.c0,Patch.flt.c0,Patch.Is,size(Patch.Is));
C = Patch.n*diag(c)-c(:)*ones(1,Patch.n);
D = A-C*Patch.B*C'/Patch.n2;


% compute Eigen vector a of D
Patch.opts.A.v0 = Patch.a;
[a,f,flag]=eigs(D,1,'sm',Patch.opts.A);
if flag ~= 0
    [a,f]=eig(D);
    [f,i]=min(diag(f));
    a = a(:,i);
    fprintf('e');
end

% invert the postive direction if it has a negative direction.
if sum(a) < 0, a=-a; end
if nargout > 2, b = -Patch.B*C'*a/Patch.n2; end

% if ~isempty(find(a < 0))
%     a = Patch.a;
%     b = Patch.b;
%     f = a'*D*a;
%     fprintf('p');
% end
f=f/2;

% verification for a and b
% sz = size(Patch.Is);
% for i = 1:Patch.n
%     Is(:,:,i) = a(i)*Patch.Is(:,:,i)+b(i);
% end
% Im = repmat(mean(Is,3),[1 1 Patch.n]);
% fq = Patch.n*sum(wnd3(Patch.flt.c0,Patch.flt.c0,(Is-Im).^2,sz))/2;
%
% c = [a; b];
% H = [A C; C' Patch.B];
% fr = c'*H*c/2;
end

% scatter matrix of all images
function A = scatter(Ix,Iy,I,J,gx,gy,nc,n,w)
I2 = Ix(:,:,I).*Iy(:,:,J);
c = wnd3(gy,gx,I2,[w w nc]);
B = sparse(I,J,c);
A = full(diag((n+1)*diag(B))-B-B');
end

% 3d convolution
function C = conv3(hcol,hrow,A,shape)
% 3-dimensional separable convolution
b1(:,1) = hcol;
C = convn(A,b1,shape);
b2(1,:) = hrow;
C = convn(C,b2,shape);
end

% 3d windowing
function b = wnd3(hcol,hrow,A,sz)
% 3-dimensional separable windowing
% b = hcol'*reshape(A,sz(1),prod(sz(2:end)));
% b = hrow'*reshape(b,sz(2),sz(end));
C = hcol*hrow';
b = C(:)'*reshape(A,sz(1)*sz(2),prod(sz(3:end)));
end

% Initial Multiple View Structure
function initMV()

global MV q;
% Filter Banks
% Correlation Window
sigma = MV.radiusPatch(1)/3;
x=[-MV.radiusPatch(1)-0.5:MV.radiusPatch(1)+0.5]';
% normpdf(x)
MV.flt.c0 = diff(normcdf(x,0,sigma));
% x*normpdf(x)
MV.flt.c1 = -diff(normpdf(x,0,sigma))*sigma^2;
% x^2*normpdf(x)
MV.flt.c2 = diff(x.*normpdf(x,0,sigma));
MV.flt.c2 = (MV.flt.c0-MV.flt.c2)*sigma^2;
% MV.flt.c0(1:MV.radiusPatch(2))=0;
% MV.flt.c0(end-MV.radiusPatch(2)+1:end)=0;
% MV.flt.c1(1:MV.radiusPatch(2))=0;
% MV.flt.c1(end-MV.radiusPatch(2)+1:end)=0;
% MV.flt.c2(1:MV.radiusPatch(2))=0;
% MV.flt.c2(end-MV.radiusPatch(2)+1:end)=0;
% MV.flt.c0 = MV.flt.c0/sum(MV.flt.c0);

cnorm = sum(MV.flt.c0);
MV.flt.c0 = MV.flt.c0/cnorm;

% Smoothing Window
sigma = MV.radiusPatch(2)/3;
x=[-MV.radiusPatch(2)-0.5:MV.radiusPatch(2)+0.5]';
% normpdf(x)
MV.flt.s0 = diff(normcdf(x,0,sigma));
snorm = sum(MV.flt.s0);
MV.flt.s0 = MV.flt.s0/snorm;
% derivative of normpdf(x)
MV.flt.s1 = diff(normpdf(x,0,sigma))/snorm;
MV.B = MV.n*eye(MV.n)-ones(MV.n);   % offset
MV.n2 = MV.n^2;                     % squared number of images

% Meshgrid of x and y
x=[-MV.radiusPatch(1):MV.radiusPatch(1)]';
[MV.X, MV.Y]=meshgrid(x);
MV.X = repmat(MV.X,[1 1 MV.n]);
MV.Y = repmat(MV.Y,[1 1 MV.n]);

MV.eps = sqrt(sqrt(eps));
x=[-1 0 1];
[MV.x1,MV.x2,MV.x3,MV.x4]=ndgrid(x,x,x,x);
MV.C = [MV.x1(:) MV.x2(:) MV.x3(:) MV.x4(:)];
MV.C = [MV.C; zeros(1,4)];

MV.w=2*MV.radiusPatch+1;   % window size

MV.scaleAngular=diag(MV.georef(1:2,1:2));
MV.scaleSpatial=MV.radiusMoon*MV.scaleAngular;
MV.a = ones(MV.n,1)/sqrt(MV.n);
MV.b = zeros(MV.n,1);

MV.N = [1:MV.n]';
C = [[MV.N MV.N]; nchoosek(MV.N,2)];
MV.I = C(:,1); MV.J = C(:,2);
MV.nc = nchoosek(MV.n,2)+MV.n;

MV.t = sqrt(3)/2;           % threshold for surface normal

MV.xdata = MV.radiusPatch(1)*[-1 1];
MV.ydata = MV.radiusPatch(1)*[-1 1];

% Elevation
MV.opts.R = optimset('Display','off');
MV.opts.R = optimset(MV.opts.R,'TolFun',0,'TolX',0);
MV.opts.R = optimset(MV.opts.R,'MaxFunEvals',120000,'MaxIter',15000);
% MV.opts.R = optimset(MV.opts.R,'PlotFcns',@optimplotfval);
% MV.opts.R = optimset(MV.opts.R,'OutputFcn', @myoutput);

% Levenberg-Marquardt Parameter
MV.opts.L = optimset('Display','off');
MV.opts.L = optimset(MV.opts.L,'TolFun',1e-5,'TolX',1e-5);
MV.opts.L = optimset(MV.opts.L,'MaxFunEvals',120,'MaxIter',100);

    function stop = myoutput(r,optimvalues,state);
        stop = false;
        %     bOptQ = false;
        %     if state == 'done',
        %         bOptQ = true;
        %         MV.opts.L = optimset(MV.opts.L,'MaxIter',100);
        %     end
        %     if strfind(optimvalues.procedure, 'parabolic'),
        %         bOptQ = true;
        %         MV.opts.L = optimset(MV.opts.L,'MaxIter',100); %optimvalues.iteration);
        %     end
        %     if bOptQ
        if state == 'done'
            MV.r=r;
            q = mpLMQuadConQ(q,MV);
        end
    end

% Surface Normal
MV.opts.Q = optimset('LargeScale','on','Display','iter');
MV.opts.Q = optimset(MV.opts.Q,'Algorithm','active-set');
MV.opts.Q = optimset(MV.opts.Q,'TolFun',1e-15,'TolX',1e-15);
% MV.opts.Q = optimset(MV.opts.Q,'PlotFcns',@optimplotfval);
MV.opts.Q = optimset(MV.opts.Q,'GradObj','on','GradConst','on');
MV.opts.Q = optimset(MV.opts.Q,'DerivativeCheck','on');
MV.opts.Q = optimset(MV.opts.Q,'MaxFunEvals',12000,'MaxIter',20000);
MV.opts.Q = optimset(MV.opts.Q,'MaxTime',100);

% Linear Reflectance
MV.opts.A.disp = 0;
end

function initMVcenter()
% Initial Multiple View Structure
global MV;

[MV.e, de] = pix2dir(MV.georef,MV.center);
for i=1:MV.n
    MV.Pe{i} = MV.camera{i}(:,1:3)*MV.e;
    MV.camcen(:,i)=-MV.camera{i}(:,1:3)\MV.camera{i}(:,4);
end
MV.height=double(MV.DEM0(MV.center(2), MV.center(1)));
MV.r = MV.radiusMoon+MV.height;
MV.r0 = MV.r;
R = [de(:,1)/norm(de(:,1)) de(:,2) MV.e];
MV.q = dcm2q(R)';

MV.rangeHeight=MV.height+MV.rangeLarge;
MV.lb = MV.radiusMoon+MV.rangeHeight(1);
MV.ub = MV.radiusMoon+MV.rangeHeight(2);
end

function drawProfile(MV,f,r,q)
h0=MV.r0-MV.radiusMoon;
f0=mpGausOptQ(MV.r0,MV);
if nargin > 2, MV.r = r; end
if nargin > 3, MV.q = q; end
MV.height = MV.r-MV.radiusMoon;

R = [MV.lb:(MV.ub-MV.lb)/20:MV.ub];
E = [];
for r = R;
    E = [E mpGausOptQ(r,MV)];
end

r = R-MV.radiusMoon;
figure, plot(r,E,'-k')
axis([min(r) max(r) 0 max(E)]);
hold on
plot(MV.height,f,'or',h0,f0,'xb')
hold off
title('Reduced Version of Linear Reflectance', 'FontSize', 14)
end

function R = vec2rot(v)
e = [1 0 0]';
u = cross(e,v);
nu = norm(u); c = e'*v;
if nu == 0
    if c > 0,
        R = eye(3);
    else
        R = -eye(3);
    end
    return;
end
u = u/nu; s = sqrt(1-c^2);
u = cross(e,v); u = u/norm(u);
c = e'*v; s = sqrt(1-c^2);
R = c*eye(3)+s*skew(u)+(1-c)*u*u';
end

function S = skew(a)
S = [0 -a(3) a(2) ; a(3) 0 -a(1); -a(2) a(1) 0];
end