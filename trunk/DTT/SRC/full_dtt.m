%% ------------------------%
%  full_dtt.m          %
%  FLow of DTT Algorithm   %
%  Author : Xifan TANG     %
%           LSI, EPFL      %
%--------------------------%

%% Function of full dtt
%  Description : a flow of DTT, calculate DTT with each source,
%                then check the results simply. Update the Denominators and 
%                Numerators for each sink. Then Calculate the transfer
%                functions
function [ stree ] = full_dtt(stree,srcidx,n,idxlst,nodes,freq,debug)

  % 1st Step: DTT by each source
  [ stree ] = dtt( srcidx,stree,debug );
  for i = 1:n
    [ stree ] = dtt( stree.bnode(idxlst(i)).next(1).nidx,stree,debug );
  end
  
  % 2nd Step: Check DTT results 
  disp(['Checking DTT results...'])
  [ stree ] = check_dtt_result(stree);
  
  % 3rd Step: Update the Denominators and Numeraters
  disp(['Update Denominators for each sink...'])
  [ stree ] = update_denom_each_sink(stree,nodes,srcidx,idxlst,debug);
  
  % 4th Step: Calculate Transfer Functions
  disp(['Calculate Transfer Functions...'])
  [ stree ] = new_cal_trans_func(stree,nodes,freq,debug);
  
  % 5th Step: Calculate Input Impedance
  disp(['Calculate Input Impedance...'])
  [ stree ] = cal_input_impedance( stree,srcidx,freq,debug );
  stree.zin = abs(stree.bnode(srcidx).imped);
  disp(['Over! Check it in stree.zin'])
end

%% Check the DTT results
%  dtt attribute of each node should equal to 2.
function [ stree ] = check_dtt_result(stree)

  % Check dtt, denominator, and numerators for each node
  for ind = 1:(stree.availnd-1)
    if ((2 ~= stree.bnode(ind).dtt)&(1 == stree.bnode(ind).btproc))
      if (1 == stree.bnode(ind).issink)
        disp(['Warning: Sink Node(Index=' num2str(ind) ') DTT(' num2str(stree.bnode(ind).dtt) ') Fail!'])
      else
        disp(['Warning: Normal Node(Index=' num2str(ind) ') DTT(' num2str(stree.bnode(ind).dtt) ') Fail!'])
      end
    end
  end

end

%% Update Denominators for each sink
function [ stree ] = update_denom_each_sink(stree,nodes,srcidx,idxlst,debug)
  % Get number of node
  numnode = size(nodes);
  for inode = 1:numnode(1)
    % Process sink node
    if (1 == stree.bnode(inode).issink)
        cur = inode;
        cursink = inode;
        if (1 == debug)
          disp(['Determine Source of Sink Node(Index=' num2str(cur) ')...'])
        end
        % Find path
        while(1)
          if (0 == stree.bnode(cur).numform)
            if (srcidx == cur)
              flag_cur = 1;
            else
              flag_cur = [srcidx idxlst] - stree.bnode(cur).form(1).nidx;
              flag_cur= sum(flag_cur==0);
            end
            if (flag_cur ~= 1)
              disp(['Warning: Sink(Index=' num2str(cursink) ') does not connected to source!(flag_cur='  num2str(flag_cur) ')'])
            else
              if (1 == debug)
                disp(['Info: Sink(Index=' num2str(cursink) ') is connected to source!'])
              end
              if (1 == debug)
                disp(['Info: Update Denominator of Node(Index=' num2str(inode) ')']) 
                disp(['with Source(Index=' num2str(cur) ')!'])
              end
              stree.bnode(inode).d = [];
              stree.bnode(inode).d = stree.bnode(cur).d;
              break;
            end
          end
          % Check connections
          cur = stree.bnode(cur).form(1).nidx; 
        end
    end
  end
end

%% Calculate the transfer functions for each sink
function [ stree ] = new_cal_trans_func(stree,nodes,freq,debug)

  disp(['Calculating the transfer function for each sink...'])
  % Get number of node
  numnode = size(nodes);
  % Define cur
  cur = 1; 
  % Clear the old information
  % Avoid the following situation happen:
  % previous stree.tfs(cur).n is a N-array
  % new stree.tfs(cur).n is a M-array. And N>M
  % Updates result in wrong stree.tfs(cur).n
  for ind = 1:numnode(1)
    % Process sink node
    inode = nodes(ind,1);
    if (1 == stree.bnode(inode).issink)
      stree.tfs(cur).idx = -1;
      stree.tfs(cur).d_org = [];
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
  for ind = 1:numnode(1)
    % Process sink node
    inode = nodes(ind,1); 
    if (1 == stree.bnode(inode).issink)
      if (1 == debug)
        disp(['Process Sink(Index=' num2str(inode) ')...'])
      end
      % Numerators
      stree.tfs(cur).idx = inode;
      % In some cases, LC bank is added to a sink.
      if (-1 ~= stree.bnode(inode).ltree.nidx)
        stree.tfs(cur).n_org = stree.bnode(stree.bnode(inode).ltree.nidx).n;
      else
        stree.tfs(cur).n_org = stree.bnode(inode).n;
      end
      stree.tfs(cur).n = truncate_zeros_array(stree.tfs(cur).n_org);
      %stree.tfs(cur).n = stree.tfs(cur).n_org;
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
      % Denominators
      % Get common denominator
      stree.tfs(cur).d_org = stree.bnode(inode).d;
      stree.tfs(cur).d = truncate_zeros_array(stree.tfs(cur).d_org);
      %stree.tfs(cur).d = stree.tfs(cur).d_org;
      stree.tfs(cur).df = stree.tfs(cur).d;
      % Get common denominator with specific freqency
      for idnm = 1:length(stree.tfs(cur).d)
        % Avoid overflow
        for iexp = 1:(idnm-1)
          stree.tfs(cur).df(idnm) = stree.tfs(cur).df(idnm)*(1i*2*pi*freq);
        end
      end
      stree.tfs(cur).dflog = log10(stree.tfs(cur).df);
      stree.tfs(cur).df_sum = sum(stree.tfs(cur).df);
      % Calculate the H(f) = N(f)/D(f)
      stree.tfs(cur).hf = abs(stree.tfs(cur).nf_sum)/abs(stree.tfs(cur).df_sum);
      if (stree.tfs(cur).hf > 1)
        disp(['Warning: Abnormal H(f)>1 Detected!(tfs Index=' num2str(cur) ')'])
      else
        if (stree.tfs(cur).hf > 1e-2)&(1 == debug)
          disp(['Attention: Potential H(f)>0.01 Detected!(tfs Index=' num2str(cur) ')'])
        end
      end
      % incremental cur
      cur = cur + 1;
    end
  end  
  disp(['Over. Check in the stree.tfs(i)'])
end


