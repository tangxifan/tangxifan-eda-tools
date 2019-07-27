clear all
close all

% Define basic string
arch_type = ["classical", "tileable"];
sb_type = ["subset", "universal", "wilton"];

% Define path variable
fdir_name_postfix = ['_fpga'];

% Define file name variable
fname_postfix = ['_W320'];

% Read source data
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
      raw_db(idata,:,:) = mdata; 
      % back to directory
      cd('..')
      % Update name tag 
      data_name(idata) = arch_type(iarch) + '\_' + sb_type(isb_type) + '\_' + sb_subtype(isb_subtype);
      % Pre-process the raw data
      db(idata,:,:) = cell2mat(raw_db(idata,:,2:end));
      % increment counter i
      idata = idata + 1; 
    end
  end
end

%% Pre-process the raw data
circuit_names = raw_db(1,:,1);
% Table the area, delay, power
area = (db(:,:,7) + db(:,:,8))'; % total area = logic_area + routing_area; 
delay = db(:,:,4)';
minW = db(:,:,2)';
wirelength = db(:,:,9)';
sb_mux_size = db(:,:,13)';
% normalize area, delay and power 
normalize_area = area./area(:,1);
normalize_delay = delay./delay(:,1);
normalize_minW = minW./minW(:,1);
normalize_wirelength = wirelength./wirelength(:,1);
normalize_sb_mux_size = sb_mux_size./sb_mux_size(:,1);
% Average the area, delay and power
average_area = mean(normalize_area);
average_delay = mean(normalize_delay);
average_minW = mean(normalize_minW);
average_wirelength = mean(normalize_wirelength);
average_sb_mux_size = mean(normalize_sb_mux_size);

%% Bar graphs
% Normailized Graph
fig_handle0 = figure;
b = bar([average_area', average_delay', average_minW']); 
ch = get(b,'children');
set(gca,'xlim',[0.5 length(data_name)+0.5],'Fontsize',16);
set(gca,'ylim',[0.75 1+0.2],'Fontsize',16);
set(gca,'XTick',1:1:length(data_name));
set(gca,'XTickLabel', data_name);
set(fig_handle0, 'Position', [1 1 2000 400]);
legend([{'Area'};{'Delay'};{'minW'}]);
ylabel('Normalized Area, Delay and minW');
xtickangle(45);
grid on

