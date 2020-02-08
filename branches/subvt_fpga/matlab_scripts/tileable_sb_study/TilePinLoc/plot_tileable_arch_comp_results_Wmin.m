clear all
close all

% Define basic string
arch_type = ["classical", "tileable"];
sb_type = ["subset", "universal", "wilton"];

% Define path variable
fdir_name_postfix = ['_fpga'];

% Define file name variable
fname_prefix = ['hetero_'];
fname_postfix = ['_minW1p3'];

% Read source data for homogeneous FPGAs
idata = 1;
for iarch = 1:length(arch_type)
  for isb_type = 1:length(sb_type)
    if ("classical" == arch_type(iarch)) 
      sb_subtype = [sb_type(isb_type)];
    else 
      sb_subtype = ["subset", "universal", "wilton"];
    end
    for isb_subtype = 1:length(sb_subtype)
      % Create file name 
      data_fname = arch_type(iarch) + '_' + sb_type(isb_type) ;
      data_fname = data_fname + '_' + sb_subtype(isb_subtype) ;
      data_fname = data_fname + fname_postfix; 
      disp('Reading file ' + data_fname)
      % change directory 
      cd(arch_type(iarch) + fdir_name_postfix);
      % Import the data file to the data base matrix 
      eval(data_fname);
      homo_raw_db(idata,:,:) = mdata; 
      % back to directory
      cd('..')
      % Update name tag 
      if ("classical" == arch_type(iarch)) 
        data_name(idata) = arch_type(iarch) + '\_' + sb_type(isb_type);
      else 
        data_name(idata) = arch_type(iarch) + '\_' + sb_type(isb_type) + '\times' + sb_subtype(isb_subtype);
      end
      % Pre-process the raw data
      homo_db(idata,:,:) = cell2mat(homo_raw_db(idata,:,2:end));
      % increment counter i
      idata = idata + 1; 
    end
  end
end

%% Pre-process the raw data for homogeneous FPGAs
homo_circuit_names = homo_raw_db(1,:,1);
% Table the area, delay, power
homo_area = (homo_db(:,:,8) + homo_db(:,:,9))'; % total area = logic_area + routing_area; 
homo_delay = homo_db(:,:,5)';
homo_minW = homo_db(:,:,2)';
homo_wirelength = homo_db(:,:,10)';
homo_sb_mux_size = homo_db(:,:,14)';
% normalize area, delay and power 
homo_normalize_area = homo_area./homo_area(:,1);
homo_normalize_delay = homo_delay./homo_delay(:,1);
homo_normalize_minW = homo_minW./homo_minW(:,1);
homo_normalize_wirelength = homo_wirelength./homo_wirelength(:,1);
homo_normalize_sb_mux_size = homo_sb_mux_size./homo_sb_mux_size(:,1);
% Average the area, delay and power
homo_average_area = mean(homo_normalize_area);
homo_average_delay = mean(homo_normalize_delay);
homo_average_minW = mean(homo_normalize_minW);
homo_average_wirelength = mean(homo_normalize_wirelength);
homo_average_sb_mux_size = mean(homo_normalize_sb_mux_size);

%% Bar graphs
% Normailized Graph
fig_handle0 = figure;
b = bar([homo_average_area', homo_average_delay', homo_average_minW']); 
ch = get(b,'children');
set(gca,'xlim',[0.5 length(data_name)+0.5],'Fontsize',16);
set(gca,'ylim',[0.75 1+0.1],'Fontsize',16);
set(gca,'XTick',1:1:length(data_name));
set(gca,'XTickLabel', data_name);
set(fig_handle0, 'Position', [1 1 2000 400]);
legend([{'Total Area'};{'Critical Path Delay'};{'W_{min}'}]);
ylabel('Normalized Area, Delay and W_{min}');
xtickangle(30);
grid on


% Read source data for heterogeneous FPGAs
idata = 1;
for iarch = 1:length(arch_type)
  for isb_type = 1:length(sb_type)
    if ("classical" == arch_type(iarch)) 
      sb_subtype = [sb_type(isb_type)];
    else 
      sb_subtype = ["subset", "universal", "wilton"];
    end
    for isb_subtype = 1:length(sb_subtype)
      % Create file name 
      data_fname = fname_prefix + arch_type(iarch) + '_' + sb_type(isb_type) ;
      data_fname = data_fname + '_' + sb_subtype(isb_subtype) ;
      data_fname = data_fname + fname_postfix; 
      disp('Reading file ' + data_fname)
      % change directory 
      cd(arch_type(iarch) + fdir_name_postfix);
      % Import the data file to the data base matrix 
      eval(data_fname);
      hetero_raw_db(idata,:,:) = mdata; 
      % back to directory
      cd('..')
      % Update name tag 
      if ("classical" == arch_type(iarch)) 
        data_name(idata) = arch_type(iarch) + '\_' + sb_type(isb_type);
      else 
        data_name(idata) = arch_type(iarch) + '\_' + sb_type(isb_type) + '\times' + sb_subtype(isb_subtype);
      end
      % Pre-process the raw data
      hetero_db(idata,:,:) = cell2mat(hetero_raw_db(idata,:,2:end));
      % increment counter i
      idata = idata + 1; 
    end
  end
end

%% Pre-process the raw data
hetero_circuit_names = hetero_raw_db(1,:,1);
% Table the area, delay, power
hetero_area = (hetero_db(:,:,8) + hetero_db(:,:,9))'; % total area = logic_area + routing_area; 
hetero_delay = hetero_db(:,:,5)';
hetero_minW = hetero_db(:,:,2)';
hetero_wirelength = hetero_db(:,:,10)';
hetero_sb_mux_size = hetero_db(:,:,14)';
% normalize area, delay and power 
hetero_normalize_area = hetero_area./hetero_area(:,1);
hetero_normalize_delay = hetero_delay./hetero_delay(:,1);
hetero_normalize_minW = hetero_minW./hetero_minW(:,1);
hetero_normalize_wirelength = hetero_wirelength./hetero_wirelength(:,1);
hetero_normalize_sb_mux_size = hetero_sb_mux_size./hetero_sb_mux_size(:,1);
% Average the area, delay and power
hetero_average_area = mean(hetero_normalize_area);
hetero_average_delay = mean(hetero_normalize_delay);
hetero_average_minW = mean(hetero_normalize_minW);
hetero_average_wirelength = mean(hetero_normalize_wirelength);
hetero_average_sb_mux_size = mean(hetero_normalize_sb_mux_size);

%% Bar graphs
% Normailized Graph
fig_handle1 = figure;
b = bar([hetero_average_area', hetero_average_delay', hetero_average_minW']); 
ch = get(b,'children');
set(gca,'xlim',[0.5 length(data_name)+0.5],'Fontsize',16);
set(gca,'ylim',[0.75 1+0.1],'Fontsize',16);
set(gca,'XTick',1:1:length(data_name));
set(gca,'XTickLabel', data_name);
set(fig_handle1, 'Position', [1 1 2000 400]);
legend([{'Area'};{'Delay'};{'W_{min}'}]);
ylabel('Normalized Area, Delay and W_{min}');
xtickangle(30);
grid on

% Merge the circuit names 
circuit_names = [homo_circuit_names hetero_circuit_names];

% Average over both homogeneous and heterogeneous FPGAs
normalize_area = [homo_normalize_area' hetero_normalize_area'];
normalize_delay = [homo_normalize_delay' hetero_normalize_delay'];
normalize_minW = [homo_normalize_minW' hetero_normalize_minW'];
normalize_wirelength = [homo_normalize_wirelength' hetero_normalize_wirelength'];
normalize_sb_mux_size = [homo_normalize_sb_mux_size' hetero_normalize_sb_mux_size'];

% Average the area, delay and power
average_area = mean(normalize_area');
average_delay = mean(normalize_delay');
average_minW = mean(normalize_minW');
average_wirelength = mean(normalize_wirelength');
average_sb_mux_size = mean(normalize_sb_mux_size');

%% Bar graphs
% Normailized Graph
fig_handle2 = figure;
b = bar([100*average_area', 100*average_delay', 100*average_minW']); 
ch = get(b,'children');
set(gca,'xlim',[0.5 length(data_name)+0.5],'Fontsize',16);
set(gca,'ylim',[85 105],'Fontsize',16);
set(gca,'XTick',1:1:length(data_name));
set(gca,'XTickLabel', data_name);
ytickformat(gca, 'percentage');
set(fig_handle2, 'Position', [1 1 2000 400]);
legend([{'Total Area'};{'Critical Path Delay'};{'W_{min}'}]);
ylabel('Normalized Area, Delay and W_{min}');
xtickangle(30);
grid on

