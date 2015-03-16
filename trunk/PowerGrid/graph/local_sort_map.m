function [ y_out z_out ] = local_sort_map( y_in, z_in )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
  if (size(y_in) ~= size(z_in))
      disp(['Error: (local_sort_map)Different size of y_in and z_in!'])
  end
  len = size(y_in);
  [y_out idx_map] = sort(y_in);
  [ z_out ] = map_ary( z_in,idx_map,len(1));
end

