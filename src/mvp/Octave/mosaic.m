function result = mosaic()
  files = ls("*.tif");

  minRow = Inf;
  maxRow = -Inf;
  minCol = Inf;
  maxCol = -Inf;
  level = 0;

  for i = 1:size(files)(1)
    [col, row, level] = sscanf(files(i, :), "%d_%d_%d", "C");
    minCol = min(col, minCol);
    maxCol = max(col, maxCol);
    minRow = min(row, minRow);
    maxRow = max(row, maxRow);
  endfor

  colOffset = minCol;
  numCols = maxCol - minCol + 1;
  rowOffset = minRow;
  numRows = maxRow - minRow + 1;

  printf("%d x %d tiles\n", numCols, numRows);

  tile_size = 64;

  result = NA(tile_size*numRows, tile_size*numCols);

  for c = 0:numCols - 1
    for r = 0:numRows - 1
      file = sprintf("%d_%d_%d.tif", c + colOffset, r + rowOffset, level);
      if (exist(file, "file"))
        tile_raw = imread_vw(file);
        tile = tile_raw(:, :, 1);
        tile(!tile_raw(:,:,2)) = NA;

        rBegin = tile_size * r;
        cBegin = tile_size * c;
        result((1:tile_size) + rBegin, (1:tile_size) + cBegin) = tile;
      endif
    endfor
  endfor

endfunction

% vim:set syntax=octave:
