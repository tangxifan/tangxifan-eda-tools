%% Calculate the Input Impedance of Tree with given frequency
function [ stree ] = cal_input_impedance( stree,srcidx,freq,debug )
  
  % With given frequency, the impedance of each branch of tree can be 
  % determined. 
  % Condition A:
  % For each sink node composed of R,L and, C, the impedance is
  % R+jwL+1/(jwC). If C equals to zero, ignore it.
  % Condition B:
  % For each branch, the impedance is R+jwL+1/(jwC+1/Zsub)
  
  % The algorthim is like DTT, which is recursive.
  if (stree.bnode(srcidx).rtree.nidx == -1)
    stree.bnode(srcidx).impedr = 0;
  else
    rcuridx = stree.bnode(srcidx).rtree.nidx;
    [ stree ] = cal_input_impedance( stree,rcuridx,freq,debug );
    stree.bnode(srcidx).impedr = stree.bnode(rcuridx).imped;
  end
      
  if (stree.bnode(srcidx).ltree.nidx == -1)
    stree.bnode(srcidx).impedl = 0;
  else
    lcuridx = stree.bnode(srcidx).ltree.nidx;
    [ stree ] = cal_input_impedance( stree,lcuridx,freq,debug );
    stree.bnode(srcidx).impedl = stree.bnode(lcuridx).imped;
  end
  
  % For debug use
  if (debug == 1)
    disp(['Cal Impedance: Curidx=' num2str(srcidx)])
  end
  
  if (stree.bnode(srcidx).impedr == 0)
    if (stree.bnode(srcidx).impedl == 0)
      stree.bnode(srcidx).imped = 0;
    else
      stree.bnode(srcidx).imped = stree.bnode(srcidx).impedl;
    end
  else
    if (stree.bnode(srcidx).impedl == 0)
      stree.bnode(srcidx).imped = stree.bnode(srcidx).impedr;
    else
      stree.bnode(srcidx).imped = 1/(1/stree.bnode(srcidx).impedr+1/stree.bnode(srcidx).impedl);
    end
  end
      
  if (stree.bnode(srcidx).c ~= 0)
    if (stree.bnode(srcidx).issink ~= 1)
      stree.bnode(srcidx).imped = 1/(1/(stree.bnode(srcidx).imped)+(1i*2*pi*freq*stree.bnode(srcidx).c));
    else
      stree.bnode(srcidx).imped = stree.bnode(srcidx).imped + 1/(1i*2*pi*freq*stree.bnode(srcidx).c);
    end
  end
  stree.bnode(srcidx).imped = stree.bnode(srcidx).imped+stree.bnode(srcidx).r;
  stree.bnode(srcidx).imped = stree.bnode(srcidx).imped+1i*2*pi*freq*stree.bnode(srcidx).l;

end

