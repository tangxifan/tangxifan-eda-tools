function [ z_sqr ] = avg_xyz( x,y,z,scale )
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

new_len = 0;
for i = 1:max(x_map)
    for j = 1:max(y_map)
        if (counter_matrix(i,j) ~= 0)
            new_len = new_len + 1;
        end
    end
end
new_x = zeros(new_len,1);
new_y = zeros(new_len,1);
new_z = zeros(new_len,1);
cur = 1;
for i = 1:max(x_map)
    for j = 1:max(y_map)
        if (counter_matrix(i,j) ~= 0)
            new_x(cur) = i;
            new_y(cur) = j;
            new_z(cur) = sum_matrix(i,j)/counter_matrix(i,j);
            cur = cur + 1;
        end
    end
end
z_sqr = grid_interp( new_x,new_y,new_z);

end