%% dtt.m
%  Direct Truncation of the Transfer Function(DTT)
%  on Clock Tree Structure
%
%  Author: Xifan TANG
%          LSI,EPFL
%

function [ stree ] = dtt( srcidx,stree,debug )
  
  disp(['DTT Start!(Source Index=' num2str(srcidx) ')'])  
  disp(['Clear all the d,m,n'])
  [ stree ] = clear_dml(stree);
  disp(['Calculating Denominator...']) 
  [ stree ] = cal_denominator( srcidx,stree,debug );
  
  %fin = stree.bnode(srcidx).n;
  fin = 1;
  disp(['Correcting Numerator...']) 
  [ stree ] = correct_numerators(srcidx,stree,fin,debug);
  disp(['DTT Over!']) 
end

%% Clear all denominator, numerator, m
%  Ensure previous results cause incorrect results
function [ stree ] = clear_dml(stree)

  for ind = 1:(stree.availnd - 1)
    stree.bnode(ind).d = [];
    stree.bnode(ind).m = [];
    stree.bnode(ind).n = [];
    stree.bnode(ind).dl = [];
    stree.bnode(ind).dr = [];
    stree.bnode(ind).ml = [];
    stree.bnode(ind).mr = [];
  end

end

%% Calculate Denominator
function [ stree ] = cal_denominator( srcidx,stree,debug )
  
  if (stree.bnode(srcidx).rtree.nidx == -1)
    stree.bnode(srcidx).dr = [1];
    stree.bnode(srcidx).mr = [0];
  else
    rcuridx = stree.bnode(srcidx).rtree.nidx;
    [ stree ] = cal_denominator( rcuridx,stree,debug );
    stree.bnode(srcidx).dr = stree.bnode(rcuridx).d;
    stree.bnode(srcidx).mr = stree.bnode(rcuridx).m;
  end
      
  if (stree.bnode(srcidx).ltree.nidx == -1)
    stree.bnode(srcidx).dl = [1];
    stree.bnode(srcidx).ml = [0];
  else
    lcuridx = stree.bnode(srcidx).ltree.nidx;
    [ stree ] = cal_denominator( lcuridx,stree,debug );
    stree.bnode(srcidx).dl = stree.bnode(lcuridx).d;
    stree.bnode(srcidx).ml = stree.bnode(lcuridx).m;
  end
  
  % For debug use
  if (debug == 1)
    disp(['Cal Denominator: Curidx=' num2str(srcidx)])
  end
  
  stree.bnode(srcidx).n = conv(stree.bnode(srcidx).dr,stree.bnode(srcidx).dl);
  stree.bnode(srcidx).m = 0;
  stree.bnode(srcidx).d = 0;
  % Prepare the calculation
  %n = truncate_zeros_array(stree.bnode(srcidx).n);
  %mldr = truncate_zeros_array(conv(stree.bnode(srcidx).ml,stree.bnode(srcidx).dr));
  %mrdl = truncate_zeros_array(conv(stree.bnode(srcidx).mr,stree.bnode(srcidx).dl));
  n = stree.bnode(srcidx).n;
  mldr = conv(stree.bnode(srcidx).ml,stree.bnode(srcidx).dr);
  mrdl = conv(stree.bnode(srcidx).mr,stree.bnode(srcidx).dl);
  % Update the maximum length of array
  maxlen = max([length(n),length(mldr),length(mrdl),length(stree.bnode(srcidx).m)]);
  % Adapt the maximum length to each part
  n = [n zeros(1,maxlen-length(n))];
  mldr = [mldr zeros(1,maxlen-length(mldr))];
  mrdl = [mrdl zeros(1,maxlen-length(mrdl))];
  stree.bnode(srcidx).m = [stree.bnode(srcidx).m zeros(1,maxlen-length(stree.bnode(srcidx).m))];
  % Do calculation
  stree.bnode(srcidx).m = stree.bnode(srcidx).c.*n + mldr+mrdl;
  
  % Prepare the calculation
  newm = conv(stree.bnode(srcidx).m,[0 stree.bnode(srcidx).r stree.bnode(srcidx).l]);
  %newm = truncate_zeros_array(newm);
  % Update the maximum length of array
  maxlen = max([maxlen,length(n),length(newm),length(stree.bnode(srcidx).d)]);
  % Adapt the maximum length to each part
  n = [n zeros(1,maxlen-length(n))];
  newm = [newm zeros(1,maxlen-length(newm))];
  stree.bnode(srcidx).d = [stree.bnode(srcidx).d zeros(1,maxlen-length(stree.bnode(srcidx).d))];
  % Do calculation
  stree.bnode(srcidx).d = n + newm;
  %stree.bnode(srcidx).d = truncate_zeros_array(stree.bnode(srcidx).d);
  % Debug purpose  
  stree.bnode(srcidx).dtt = 1;
  
  % Debug Info
  if (debug == 1)
    disp(['INFO: index=' num2str(srcidx)])
    disp(['INFO: dl=' num2str(stree.bnode(srcidx).dl)])
    disp(['INFO: dr=' num2str(stree.bnode(srcidx).dr)])
    disp(['INFO: ml=' num2str(stree.bnode(srcidx).ml)])
    disp(['INFO: mr=' num2str(stree.bnode(srcidx).mr)])
    disp(['INFO: mldr=' num2str(mldr)])
    disp(['INFO: mrdl=' num2str(mrdl)])
    disp(['INFO: N=' num2str(stree.bnode(srcidx).n)])
    disp(['INFO: M=' num2str(stree.bnode(srcidx).m)])
    disp(['INFO: D=' num2str(stree.bnode(srcidx).d)])
  end
  
end

%% Correct Numerators
function [ stree ] = correct_numerators(srcidx,stree,fin,debug)
  
  ridx = stree.bnode(srcidx).rtree.nidx;
  lidx = stree.bnode(srcidx).ltree.nidx;
  
  if (ridx ~= -1)
    fr = conv(fin,stree.bnode(srcidx).dl);
    [ stree ] = correct_numerators(ridx,stree,fr,debug);
  end

  if (lidx ~= -1)
    fl = conv(fin,stree.bnode(srcidx).dr);
    [ stree ] = correct_numerators(lidx,stree,fl,debug);
  end

  newn = conv(stree.bnode(srcidx).n,fin);
  stree.bnode(srcidx).n = [];
  stree.bnode(srcidx).n = newn;
  % debug purpose 
  stree.bnode(srcidx).dtt = stree.bnode(srcidx).dtt + 1;
  if ((1 == debug)&&(1 == stree.bnode(srcidx).issink))
    disp(['DEBUG: Correct Sink Numerator(Index=' num2str(srcidx) ')'])
  end

end


