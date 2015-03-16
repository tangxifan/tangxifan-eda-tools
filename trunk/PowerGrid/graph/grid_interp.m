function [ z_sqr ] = grid_interp( x,y,z)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
%% Interpolate the matrix gnd
xi_col = 1:max(x);
yi_row = 1:max(y);
[xi,yi] = meshgrid(xi_col,yi_row);
% Interpolate 2-D
z_sqr = griddata(x,y,z,xi,yi,'cubic');
z_sqr = z_sqr';
end

