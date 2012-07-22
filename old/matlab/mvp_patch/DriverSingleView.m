function DriverSingleView
clc; close all;

strIn = '../../real_scene/AS15_3_3_tiles.mat';
load(strIn);

    sv=SingleView(images(1).data/2^17,images(1).camera);
    sv.disp;
    sv.q=rand(4,1);
end

