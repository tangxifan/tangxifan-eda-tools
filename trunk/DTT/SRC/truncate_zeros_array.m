%% Function Truncate the useless zeros at the end of array
function [ array_out ] = truncate_zeros_array(array_in)
  zero_flag = (array_in ~= 0);
  nonzero_idx = strfind(zero_flag,1);
  final_index = max(nonzero_idx);
  array_out(1:final_index) = array_in(1:final_index);
end