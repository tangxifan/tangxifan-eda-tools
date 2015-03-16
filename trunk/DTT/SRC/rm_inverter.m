
%% ----------------------------%
%   rm_inverter.m              %
%   remove inverter from node  %
%   Author : Xifan TANG        %
%            LSI, EPFL         %
%------------------------------%

%% function rm_inverter
%  With given node list, remove inverters from these nodes
function [ stree ] = rm_inverter(stree,n,nidxes,debug)
  
  % Remove inverter one by one 
  disp(['Remove Inverters...'])
  for i = n:-1:1
    if (debug == 1)
      disp(['Remove Inverter(Index=' num2str(nidxes(i)) ')'])
    end
    [ stree ] = rm_inv_node(stree,nidxes(i),debug);
  end
  disp(['Over'])

end

%% function rm_inv_node, remove inverter from specific node
function [ stree ] = rm_inv_node(stree,nidx,debug)

  %  Assume, Node 1 is nidx, Node 4 is inverter node.
  %  Node 2,3 are sub-nodes
  %      Previous         After
  %          1              1
  %          |             / \
  %          4            2   3
  %         / \       
  %        2   3

  inv_node = stree.bnode(nidx).next(1).nidx;
  % 1st Step: Configure Node 1
  % Remove Cin of Inverter
  stree.bnode(nidx).c = stree.bnode(nidx).c - stree.bnode(nidx).ci_inv;
  % Recover the connections
  % Nexter node
  stree.bnode(nidx).numnext = stree.bnode(inv_node).numnext;
  for icur = 1:stree.bnode(inv_node).numnext
    stree.bnode(nidx).next(icur).nidx = stree.bnode(inv_node).next(icur).nidx;
  end
  % Left&Right Tree
  stree.bnode(nidx).ltree.nidx = stree.bnode(inv_node).ltree.nidx;
  stree.bnode(nidx).rtree.nidx = stree.bnode(inv_node).rtree.nidx;
  
  % 2nd Step: Configure Node 2,3
  lidx = stree.bnode(nidx).ltree.nidx;  
  ridx = stree.bnode(nidx).rtree.nidx; 
  if (lidx ~= -1)
    stree.bnode(lidx).form(1).nidx = nidx;
  end
  if (ridx ~= -1)
    stree.bnode(ridx).form(1).nidx = nidx;
  end

  % 3rd Step: Initialize inverter node
  % Clear Basic Information
  stree.bnode(inv_node).btproc = 0;
  stree.bnode(inv_node).x = 0;
  stree.bnode(inv_node).y = 0;
  stree.bnode(inv_node).cl = 0;
  % Clear RLC info
  stree.bnode(inv_node).r = 0;
  stree.bnode(inv_node).l = 0;
  stree.bnode(inv_node).c = 0;
  % Clear Sink Info
  stree.bnode(inv_node).issink = 0; 
  stree.bnode(inv_node).sidx = -1; 
  % Former node
  stree.bnode(inv_node).numform = 0;
  stree.bnode(inv_node).form = [];
  % Nexter node
  stree.bnode(inv_node).numnext = 0;
  stree.bnode(inv_node).next = [];
  % Left&Right Tree
  stree.bnode(inv_node).ltree = [];
  stree.bnode(inv_node).rtree = [];
  % Clear d,m,l,dtt
  stree.bnode(inv_node).d = [];
  stree.bnode(inv_node).n = [];
  stree.bnode(inv_node).m = [];
  stree.bnode(inv_node).dtt = [];
  
  % debug
  if (1 == debug)
    disp(['Remove Inverter Node(Index=' num2str(inv_node) ')']) 
  end

  if (stree.availnd == (inv_node+1))
    stree.availnd = stree.availnd - 1;
    disp(['Free Node(Availnd=' num2str(stree.availnd) ')'])
  end

end

