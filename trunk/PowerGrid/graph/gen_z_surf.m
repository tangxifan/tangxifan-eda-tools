function [ z_sqr ] = gen_z_surf( x,y,z,num )
%% Process the VDD data
% sort the x_vdd and map the y_vdd and z_vdd
x_vdd_temp = sort(x);
y_vdd_temp = sort(y);
% 
x_temp = x_vdd_temp(1);
y_temp = y_vdd_temp(1);
i = 1;
% row -> y
% col -> x
nrow = 1;
ncol = 1;
while(i <= num)
    if x_temp ~= x_vdd_temp(i)
      x_temp = x_vdd_temp(i);
      ncol = ncol + 1;
    end
    if y_temp ~= y_vdd_temp(i)
      y_temp = y_vdd_temp(i);
      nrow = nrow + 1;
    end
    i = i + 1;
end
%% Retraversal the array and generate the map of index
z_surf = zeros(nrow,ncol);
% the array for index mapping
x_vdd_idx = zeros(ncol,1);
y_vdd_idx = zeros(nrow,1);
% 
x_temp = x_vdd_temp(1);
y_temp = y_vdd_temp(1);
i = 1;
%
x_vdd_idx(1) = x_temp;
y_vdd_idx(1) = y_temp;
% current column and row
x_cur = 2;
y_cur = 2;
%
while(i <= num)
    if x_temp ~= x_vdd_temp(i)
      x_vdd_idx(x_cur) = x_vdd_temp(i);
      x_cur = x_cur + 1;
      x_temp = x_vdd_temp(i);
    end
    if y_temp ~= y_vdd_temp(i)
      y_vdd_idx(y_cur) = y_vdd_temp(i);
      y_cur = y_cur + 1;
      y_temp = y_vdd_temp(i);
    end
    i = i + 1;
end
%% According to the index map, Build the z_matrix
i = 1;
for i = 1:num
  x_idx = find_idx(x_vdd_idx,x(i),ncol);
  y_idx = find_idx(y_vdd_idx,y(i),nrow);
  z_surf(x_idx,y_idx) = z(i);
end
%% Enlarge the matrix to a square
nsqr = max(nrow,ncol);
z_sqr = zeros(nsqr);
% If the nrow > ncol, which means the y > x, 
% so we need to enlarge the x axis
if (nrow > ncol)
   intvl = floor((nrow - ncol)/ncol) + 1;
   cur = 1;
   for i = 1:ncol
       z_sqr(cur,:) = z_surf(i,:);
       cur = cur + intvl;
   end
else
   intvl = floor((ncol - nrow)/nrow) + 1;
   cur = 1;
   for i = 1:nrow
       z_sqr(:,cur) = z_surf(:,i);
       cur = cur + intvl;
   end
end
%% Interpolate the matrix
xi_col = 1:1:nsqr;
yi_row = 1:1:nsqr;
[xi,yi] = meshgrid(xi_col,yi_row);
% Interpolate 2-D
z_sqr = interp2(z_sqr,xi,yi,'cubic');
%% Print out the graph
%mesh(z_sqr);
%view(2);

end

