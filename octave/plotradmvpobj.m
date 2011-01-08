demPt = [20; 20];
halfKern = 7;

n = 20;
planeLonlat = [0.98447 0.16892];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

patchLon = ccrop(demLon, pt, halfKern);
patchLat = ccrop(demLat, pt, halfKern);

planeRads = linspace(1734790, 1734797, n);

o = zeros([n 1]);
for k = 1:n
  obj = mvpobj_gauss([planeLonlat planeRads(k)], patchLon, patchLat, orbits)
  o(k) = obj;
  disp(k);
endfor

plot(planeRads, o);
