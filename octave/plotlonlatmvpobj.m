demPt = [20; 20];
halfKern = 7;

n = 20;
rad = 1734793.07299805;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

patchLon = ccrop(demLon, pt, halfKern);
patchLat = ccrop(demLat, pt, halfKern);

[planeLons planeLats] = meshgrid(linspace(0, 2 * pi, n));

o = zeros(n);
for r = 1:n
  for c = 1:n
    obj = mvpobj_gauss([planeLons(r, c) planeLats(r, c) rad], patchLon, patchLat, orbits)
    o(r, c) = obj;
  endfor
  disp(r);
endfor

imagesc(o);
