function [ rt_idx ] = find_idx( in_ary, target, len )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
rt_idx = -1;
 for i = 1:len
     if ( target == in_ary(i))
         rt_idx = i;
     end
 end

end

