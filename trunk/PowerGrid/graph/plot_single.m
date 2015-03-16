clc
clear all
opengl neverselect;
%% Read the file
fid = fopen('../solutions/ibmpg6.log','r');
temp = 0;
cur = 0;
lines = 0;
num_vdd = 0;
num_gnd = 0;
num_pack_vdd = 0;
num_pack_gnd = 0;
while (temp ~= (-1))
  temp = fgetl(fid);
  %disp(['line' temp])
  if ((-1) == temp)
    break;
  end
  lines = lines + 1;
  if (temp(1) ~= '*')
    [linfo  count err next] = sscanf (temp, '%*s %s %*s',1);
    if ('pac' == linfo(1:3))
      [linfo1  count err next1] = sscanf (temp(next:end), '%*s',1);
      [linfo2  count] = sscanf (temp(next+next1:end), '%d %d %f');
      if ( 0 == linfo2(3))
        num_pack_gnd = num_pack_gnd + 1;
      else
        num_pack_vdd = num_pack_vdd + 1;
      end
    end
    if ('Y' == temp(end))
      if ('vdd' == linfo(1:3))
        num_vdd = num_vdd + 1;
      end  
      if  ('gnd' == linfo(1:3))
        num_gnd = num_gnd + 1;
      end
    end
  end 
end
%% show the number of lines and nodes
disp(['Number of lines = ' num2str(lines)])
disp(['Number of vdd nodes = ' num2str(num_vdd)])
disp(['Number of gnd nodes = ' num2str(num_gnd)])
disp(['Number of package_vdd nodes = ' num2str(num_pack_vdd)])
disp(['Number of package_gnd nodes = ' num2str(num_pack_gnd)])
%% Filter the data to plot a graph (VDD)
ivdd = 1;
ignd = 1;
ipvdd = 1;
ipgnd = 1;
% VDD
x_vdd = zeros(num_vdd,1);
y_vdd = zeros(num_vdd,1);
z_vdd = zeros(num_vdd,1);
% GND
x_gnd = zeros(num_gnd,1);
y_gnd = zeros(num_gnd,1);
z_gnd = zeros(num_gnd,1);
% Package VDD
x_pack_vdd = zeros(num_pack_vdd,1);
y_pack_vdd = zeros(num_pack_vdd,1);
z_pack_vdd = zeros(num_pack_vdd,1);
% Package GND
x_pack_gnd = zeros(num_pack_gnd,1);
y_pack_gnd = zeros(num_pack_gnd,1);
z_pack_gnd = zeros(num_pack_gnd,1);
%% rescan the file
frewind(fid);
while(1)
  temp =  fgetl(fid);
  if ((-1) == temp)
      break;
  end
  if (temp(1) ~= '*')
    [linfo  count err next] = sscanf (temp, '%*s %s %*s',1);
    if ('pac' == linfo(1:3))
      [linfo1  count err next1] = sscanf (temp(next:end), '%*s',1);
      [linfo2  count] = sscanf (temp(next+next1:end), '%d %d %f');
      if ( 0 == linfo2(3))
        x_pack_gnd(ipgnd) = linfo2(1);
        y_pack_gnd(ipgnd) = linfo2(2);
        z_pack_gnd(ipgnd) = linfo2(3);
        ipgnd = ipgnd + 1;
      else
        x_pack_vdd(ipvdd) = linfo2(1);
        y_pack_vdd(ipvdd) = linfo2(2);
        z_pack_vdd(ipvdd) = linfo2(3);
        ipvdd = ipvdd + 1;
      end
    elseif ('Y' == temp(end))
      if ('vdd' == linfo(1:3))
        %disp(['Match...Add to vector...'])
        [linfo1  count err next1] = sscanf (temp(next:end), '%*s',1);
        [linfo2  count] = sscanf (temp(next+next1:end), '%d %d %f');
        x_vdd(ivdd) = linfo2(1);
        y_vdd(ivdd) = linfo2(2);
        z_vdd(ivdd) = linfo2(3);
        ivdd = ivdd + 1;  
      elseif ('gnd' == linfo(1:3))
        %disp(['Match...Add to vector...'])
        [linfo1  count err next1] = sscanf (temp(next:end), '%*s',1);
        [linfo2  count] = sscanf (temp(next+next1:end), '%d %d %f');
        x_gnd(ignd) = linfo2(1);
        y_gnd(ignd) = linfo2(2);
        z_gnd(ignd) = linfo2(3);
        ignd = ignd + 1;
      end
    end
  end
end
fclose(fid);
%%
scale = 5;
x_min = min([min(round(x_pack_gnd/scale)),min(round(x_gnd/scale)),min(round(x_pack_vdd/scale)),min(round(x_vdd/scale))]);
x_max = max([max(round(x_pack_gnd/scale)),max(round(x_gnd/scale)),max(round(x_pack_vdd/scale)),max(round(x_vdd/scale))]);
y_min = min([min(round(y_pack_gnd/scale)),min(round(y_gnd/scale)),min(round(y_pack_vdd/scale)),min(round(y_vdd/scale))]);
y_max = max([max(round(y_pack_gnd/scale)),max(round(y_gnd/scale)),max(round(y_pack_vdd/scale)),max(round(y_vdd/scale))]);
%% Plot the surf for gnd
z_sqr_gnd = avg_xyz( x_gnd,y_gnd,z_gnd,scale);
figure(1);
surf(z_sqr_gnd);
title('Gnd');
axis([x_min x_max y_min y_max]);
shading flat;
view(2);
%% Plot the surf for vdd 
new_z_vdd = max(z_pack_vdd) - z_vdd;
z_sqr_vdd = avg_xyz( x_vdd,y_vdd,new_z_vdd,scale);
figure(2);
surf(z_sqr_vdd);
shading flat;
axis([x_min x_max y_min y_max]);
title('deltaVDD(PackageVdd-Vdd)');
view(2);
%% Line the package nodes GND
map_x_pgnd = round(x_pack_gnd/scale);
map_y_pgnd = round(y_pack_gnd/scale);
figure(3);
for i = 1:length(map_x_pgnd)
  line(map_x_pgnd(i),map_y_pgnd(i),'LineWidth',2,'MarkerEdgeColor','k','Marker','d','MarkerSize',10)
end
axis([x_min x_max y_min y_max]);
title('Gnd Package Node');
%% Line the package nodes VDD
map_x_pvdd = round(x_pack_vdd/scale);
map_y_pvdd = round(y_pack_vdd/scale);
figure(4);
for i = 1:length(map_x_pvdd)
  line(map_x_pvdd(i),map_y_pvdd(i),'LineWidth',2,'MarkerEdgeColor','k','Marker','d','MarkerSize',10)
end
axis([x_min x_max y_min y_max]);
title('Vdd Package Node');
%% Interpolate the matrix vdd
%z_sqr = grid_interp( x_vdd,y_vdd,z_vdd,scale );
%z_sqr = avg_xyz( x_vdd,y_vdd,z_vdd,5);
%surf(z_sqr);
%shading flat;
%view(2);
%hold on
%z_sqr = surf_uninterp( x_vdd,y_vdd,z_vdd,50);
