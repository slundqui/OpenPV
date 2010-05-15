
function [fh] = pvp_reconstruct( recon_array, plot_title, fh, ...
				size_recon)
  
  global NK NO NROWS NCOLS
  global ROTATE_FLAG
  global MIN_INTENSITY
  global FLAT_ARCH_FLAG
  global NUM2STR_FORMAT
  global output_path

  if ~exist('NUM2STR_FORMAT') || isempty(NUM2STR_FORMAT)
    NUM2STR_FORMAT = "%03.3i";
  endif
  
  if ~exist('MIN_INTENSITY') || isempty(MIN_INTENSITY)
    MIN_INTENSITY = 0;
  endif
  
  if ~exist('DTH', 'var') || isempty(DTH)
    DTH = 180 / NO;
  endif

  if ~any(recon_array(:) ~= 0.0) % any(A) tests whether any of the elements
    return;
  endif

				% tests if 'fh' is a variable in the workspace

  if ~exist('size_recon', 'var') || isempty(size_recon) || nargin < 4
    size_recon = [NK NO NCOLS NROWS];
  endif
  NK = size_recon(1);
  NO  = size_recon(2);
  NCOLS  = size_recon(3);
  NROWS = size_recon(4);

 % disp(['size_recon = ', num2str(size_recon)]);

  ave_recon = mean(recon_array(recon_array ~= 0.0));
%  if ave_recon < 0
%    recon_array = -recon_array;
%    ave_recon = -ave_recon;
%  endif
  max_recon = max(recon_array(:));
  min_recon = min(recon_array(:));

				% if activity is sparse, plot all pixels, else only plot pixels > mean
  log2_size = max( log2(NROWS), log2(NCOLS) );
  nz_recon = length(find(recon_array ~= 0.0));
  if nz_recon < length(recon_array) / (((log2_size-4)*(log2_size > 4) + (log2_size>4))*NO*NK)
    min_recon_val = min_recon;
  else
    min_recon_val = min( (max_recon - ave_recon) / 2, 0 );
  endif
  if min_recon_val == max_recon
    min_recon_val = 0;
  endif
  max_recon_val = max_recon;
				%min_recon_val = max( 0, ave_recon );
  disp(['min_recon_val = ', num2str(min_recon_val)]);
  disp(['max_recon_val = ', num2str(max_recon_val)]);
       
  edge_len = sqrt(2)/2;
  if (NO==1)
    edge_len = sqrt(2)/2;
  endif
  if log2_size > 5
    edge_len = (log2_size - 5) * edge_len;
  endif
    
  theta_offset = 0.5 * ROTATE_FLAG;
  max_line_width = 2.5;
  axis([0 NCOLS+1 0 NROWS+1]);
  axis square;
  axis tight
  box on
  axis on
				%colorbar('East');
  colormap('gray');
				% cmap = colormap;
  hold on;
  if ~FLAT_ARCH_FLAG && NO > 1
    
    if ~exist('fh','var') || isempty(fh) || nargin < 3
      fh = figure;
    endif
    set(fh, 'Name', plot_title);

    [recon_array, recon_ndx] = sort(recon_array(:));
    last_recon_ndx = find(recon_array < 0, 1, 'last');
    first_recon_ndx = find(recon_array > 0, 1, 'first');
    if isempty(first_recon_ndx)
      first_recon_ndx = 1;
    else
      last_recon_ndx = length(recon_array);
    endif
    for recon_index = first_recon_ndx : last_recon_ndx
      i_recon = recon_ndx(recon_index);
      recon_val = recon_array(recon_index);
      if recon_val <= min_recon_val
	continue;
      else
	recon_val = ( recon_val - min_recon_val ) / ( max_recon_val - min_recon_val + (max_recon_val == min_recon_val) );
      endif
      [i_k, i_theta, j_col, i_row] = ind2sub( size_recon, i_recon );
      clear i_k
      i_theta = i_theta - 1 + theta_offset;
      j_col = j_col;
      i_row = i_row;
      delta_col = edge_len * ( cos(i_theta * DTH * pi / 180 ) );
      delta_row = edge_len * ( sin(i_theta * DTH * pi / 180 ) );
      lh = line( [j_col - delta_col, j_col + delta_col], ...
		[(i_row - delta_row), (i_row + delta_row)] );
      line_width = MIN_INTENSITY + (1-MIN_INTENSITY) * max_line_width * recon_val;
      set( lh, 'LineWidth', line_width );
      line_color = 1 - recon_val;
      set( lh, 'Color', line_color*(1-MIN_INTENSITY)*[1 1 1]);
    endfor
  elseif ~FLAT_ARCH_FLAG && NO == 1
    fh = zeros(1,NFEATURES);
    fh = figure;
    recon2D = reshape( recon_array(:), [NCOLS, NROWS]' );
    set(fh, 'Name', plot_title);
    imagesc( recon2D );  % plots recod2D as an image
    colormap('gray');
   elseif FLAT_ARCH_FLAG
    NFEATURES = size_recon(1) * size_recon(2);
    fh = [];
    recon3D = reshape( recon_array(:), [NFEATURES, NCOLS, NROWS] );
    recon3D = ( recon3D - min_recon_val ) ./ ( max_recon_val - min_recon_val + (max_recon_val == min_recon_val) );
    recon3D = uint8(255*recon3D);
    for i_feature = 1 : NFEATURES
      plot_title_tmp = ...
	  [output_path, plot_title, '_', num2str(i_feature, NUM2STR_FORMAT), '.tiff'];
      imwrite( squeeze( recon3D(i_feature,:,:) )', ...
	      plot_title_tmp, 'tiff');
    endfor
  endif
  hold off;
