% cntrPt is in x,y (not r,c)
function r = ccrop(im, cntrPt, halfKern)
  r = im(cntrPt(2)-halfKern:cntrPt(2)+halfKern, cntrPt(1)-halfKern:cntrPt(1)+halfKern);
endfunction
