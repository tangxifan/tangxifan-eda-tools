
%% Cal_transfer_func.m
% Author : Xifan TANG
% Organization:  EPFL 
% Description : The denominator of each sink node should be same.
%               Actually, they share the common denominator of
%               the source node.
%               Then with the given working frequency, 
%               the coefficients of denominators and numerators
%               are calculated.

%% Subroutines
function [ stree ] = cal_transfer_func( stree,nodes,srcidx,freq )
%   Calculate the transfer function with Denominator and Numerator
 
  disp(['Calculating the transfer function for each sink...'])
  % Get number of node
  numnode = size(nodes);
 
  % Get common denominator
  denom_org = stree.bnode(srcidx).d; 
  denom_trunc = truncate_zeros_array(denom_org);
  denomf = denom_trunc;
  % Get common denominator with specific freqency
  
  for idnm = 1:length(denom_trunc)
    % Avoid overflow
    for iexp = 1:(idnm-1)
      denomf(idnm) = denomf(idnm)*(1i*2*pi*freq);
    end
  end
  denomflog = log10(denomf);
 
  % Define cur
  cur = 1; 
  % Clear the old information
  % Avoid the following situation happen:
  % previous stree.tfs(cur).n is a N-array
  % new stree.tfs(cur).n is a M-array. And N>M
  % Updates result in wrong stree.tfs(cur).n
  for inode = 1:numnode(1)
    % Process sink node
    if (1 == stree.bnode(inode).issink)
      stree.tfs(cur).idx = -1;
      stree.tfs(cur).d = [];
      stree.tfs(cur).df = [];
      stree.tfs(cur).df_sum = [];
      stree.tfs(cur).dflog = [];
      stree.tfs(cur).n_org = [];
      stree.tfs(cur).n = [];
      stree.tfs(cur).nf = [];
      stree.tfs(cur).nf_sum = [];
      stree.tfs(cur).nflog = [];
      % incremental cur
      cur = cur + 1;
    end
  end

  % Give values
  cur = 1;
  for inode = 1:numnode(1)
    % Process sink node
    if (1 == stree.bnode(inode).issink)
      stree.tfs(cur).idx = inode;
      stree.tfs(cur).d = denom_trunc;
      stree.tfs(cur).df = denomf;
      stree.tfs(cur).df_sum = sum(denomf);
      stree.tfs(cur).dflog = denomflog;
      stree.tfs(cur).n_org = stree.bnode(inode).n;
      stree.tfs(cur).n = truncate_zeros_array(stree.tfs(cur).n_org);
      % Get numerator with specific freqency
      for inm = 1:length(stree.tfs(cur).n)
        stree.tfs(cur).nf(inm) = stree.tfs(cur).n(inm);
        % Avoid overflow
        for iexp = 1:(inm-1)
          stree.tfs(cur).nf(inm) = stree.tfs(cur).nf(inm) *(1i*2*pi*freq);
        end
      end
      stree.tfs(cur).nf_sum = sum(stree.tfs(cur).nf);
      stree.tfs(cur).nflog = log10(stree.tfs(cur).nf);
      % incremental cur
      cur = cur + 1;
    end
  end  
  disp(['Over. Check in the stree.tfs(i)'])
end


