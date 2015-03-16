
%% rinsert_lc_bank.m
%  Add LC Tanks 
%  Author: Xifan TANG
%          LSI,EPFL
%
function [ stree ] = rinsert_lc_bank(stree,indirlen,indircands,rthres,indirr,indirl,indirc,debug)

  % Add LC banks with given node list
  disp(['Adding Indirect LC banks...'])
  for i = 1:indirlen
    if (debug == 1)
      disp(['Add Indirect LC bank(NodeIndex=' num2str(indircands(i)) ' r=' num2str(indirr(i)) ' l=' num2str(indirl(i)) ' c=' num2str(indirc(i)) ')'])
    end
    [ stree ] = rinsert_lc_bank_node(stree,indircands(i),rthres,indirr(i),indirl(i),indirc(i),debug);
  end
  disp(['Over'])
end

%% Add LC Tank to a specific node
function [ stree ] = rinsert_lc_bank_node(stree,nodeidx,rthres,rval,lval,cval,debug)

  % Add an additional subtree at specific nodeidx 
  % Condition 1: 
  %       1          1
  %      / \        / \
  %     2   3      4   5
  %               / \
  %              2   3
  % Node 1 is the nodeidx, whose sub-nodes are node 2 and 3.
  % Add an extra node 4 as shadow node and node 5 as LC bank.
  
  % Get the available node index
  former = stree.bnode(nodeidx).form(1).nidx;
  rinser = rthres - stree.bnode(former).accumr;
  ratio = rinser/stree.bnode(nodeidx).r;
  linser = ratio*stree.bnode(nodeidx).l;
  cinser = ratio*stree.bnode(nodeidx).c;
  
  rcur = stree.bnode(nodeidx).r - rinser;
  lcur = stree.bnode(nodeidx).l*(1-ratio);
  ccur = stree.bnode(nodeidx).c*(1-ratio);

  newnode = stree.availnd;
  lcbank = newnode + 1;
  
  if (1 == debug)
    disp(['New Sub node created(Index=' num2str(newnode) ')'])
  end
  % Configure the new node
  % Initial the new node
  stree.bnode(newnode).btproc = 1;
  stree.bnode(newnode).x = 0;
  stree.bnode(newnode).y = 0;
  stree.bnode(newnode).cl = 0;
  % RLC info
  stree.bnode(newnode).r = rinser;
  stree.bnode(newnode).l = linser;
  stree.bnode(newnode).c = cinser;
  % sink info
  stree.bnode(newnode).issink = 0;
  stree.bnode(newnode).sidx = -1;
  % former node
  stree.bnode(newnode).numform = 1;
  stree.bnode(newnode).form(1).nidx = former;
  % Nexter node
  stree.bnode(newnode).numnext = 2;
  stree.bnode(newnode).next(1).nidx = nodeidx;
  stree.bnode(newnode).next(2).nidx = lcbank;
  
  % Copy the left and right tree from nodeidx to new node
  stree.bnode(newnode).ltree.nidx = nodeidx;
  stree.bnode(newnode).rtree.nidx = lcbank;
  
  % 2nd Step : Configure the former node
  if (stree.bnode(former).next(1).nidx == nodeidx)
    stree.bnode(former).next(1).nidx = newnode;
  else
    stree.bnode(former).next(2).nidx = newnode;
  end

  if (stree.bnode(former).ltree.nidx == nodeidx)
    stree.bnode(former).ltree.nidx = newnode;
  else
    stree.bnode(former).rtree.nidx = newnode;
  end
  
  % 3rd Step : Configure the nodeidx
  stree.bnode(nodeidx).form(1).nidx = newnode;
  stree.bnode(nodeidx).r = rcur; 
  stree.bnode(nodeidx).l = lcur; 
  stree.bnode(nodeidx).c = ccur; 
  
  % Create a new node for LC bank
  if (1 == debug)
    disp(['LC bank node created(Index=' num2str(lcbank) ')'])
  end
  % Configure the lcbank node
  stree.bnode(lcbank).btproc = 1;
  stree.bnode(lcbank).x = 0;
  stree.bnode(lcbank).y = 0;
  stree.bnode(lcbank).cl = cval;
  % RLC info
  stree.bnode(lcbank).r = rval;
  stree.bnode(lcbank).l = lval;
  stree.bnode(lcbank).c = cval;
  % sink info
  stree.bnode(lcbank).issink = 1;
  stree.bnode(lcbank).sidx = 'rlcbank';
  % former node
  stree.bnode(lcbank).numform = 1;
  stree.bnode(lcbank).form(1).nidx = newnode;
  % Nexter node
  stree.bnode(lcbank).numnext = 0;
  % Configure Left and Right Tree
  stree.bnode(lcbank).ltree.nidx = -1;
  stree.bnode(lcbank).rtree.nidx = -1;

  % Update the stree availnd
  stree.availnd = lcbank + 1;
  
end

