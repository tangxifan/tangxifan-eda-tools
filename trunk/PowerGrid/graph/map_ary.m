function [ rt_ary ] = map_ary( in_ary,idx_map,len_ary )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
  ary_temp = zeros(len_ary,1);
  for i = 1:len_ary
   ary_temp(i) = in_ary(idx_map(i));
  end
  rt_ary = ary_temp;
end

