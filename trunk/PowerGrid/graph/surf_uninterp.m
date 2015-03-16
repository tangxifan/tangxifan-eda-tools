function [ surf_m ] = surf_uninterp( x,y,z,scale )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
len = length(x);
% Update the Voltage Matrix
x_map = round(x/scale);
y_map = round(y/scale);
sum_matrix = zeros(max(x_map),max(y_map));
counter_matrix = zeros(max(x_map),max(y_map));
for i = 1:length(x_map)
  sum_matrix(x_map(i),y_map(i)) = sum_matrix(x_map(i),y_map(i)) + z(i);
  counter_matrix(x_map(i),y_map(i)) = counter_matrix(x_map(i),y_map(i)) + 1;
end
temp_m = (counter_matrix == 0);
temp_m = (temp_m + counter_matrix);
surf_m = sum_matrix./temp_m;
surf(surf_m);
shading flat;
view(2);
hold on;
end

