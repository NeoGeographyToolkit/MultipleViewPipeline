function [xw yw] = _porthoproj_impl(planeNormal, planeD, demPt, georef, orbit, hWin)
  dim = 2 * hWin + 1;

  cntrPtLonLatH = georef * [demPt(1); demPt(2); 1];
  [e de_d0] = lonlat2normal(cntrPtLonLatH(1:2) / cntrPtLonLatH(3));

  H1 = orbit.cam * [eye(3,3);planeNormal'/planeD];
  dH1_de = (H1(1:2,:) - [H1(1,:)*e;H1(2,:)*e]*H1(3,:)) / (H1(3,:) * e);
  S = dH1_de * de_d0 * georef(1:2, 1:2);
  
  cntrPtOrbH = H1 * e;
  cntrPtOrb = cntrPtOrbH(1:2) / cntrPtOrbH(3);

  for r = 1:dim
    for c = 1:dim
       off = S * ([c; r] - hWin - 1);

       pt = cntrPtOrb + off;
       xw(r, c) = pt(1);
       yw(r, c) = pt(2);
%      ptH = H * [c; r; 1];
%      xw(r, c) = ptH(1) / ptH(3);
%      yw(r, c) = ptH(2) / ptH(3);
    endfor
  endfor
endfunction

  %[X, Y] = meshgrid(1:dim);
  %D = [X(:), Y(:), ones(dim * dim, 1)]';
  %PD = H * D;
  %XW = PD(1,:)./PD(3,:);
  %YW = PD(2,:)./PD(3,:);
  %xw = reshape(XW, dim, dim);
  %yw = reshape(XW, dim, dim);


