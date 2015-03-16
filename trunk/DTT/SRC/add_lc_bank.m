
%% add_lc_bank.m
%  Add LC Tanks 
%  Author: Xifan TANG
%          LSI,EPFL
%
function [ stree ] = add_lc_bank( stree,n,nidxes,rls,lls,cls,debug )

  % Add LC banks with given node list
  disp(['Adding Dircect LC banks...'])
  for i = 1:n
    if (debug == 1)
      disp(['Add Direct LC bank(NodeIndex=' num2str(nidxes(i)) ' r=' num2str(rls(i)) ' l=' num2str(lls(i)) ' c=' num2str(cls(i)) ')'])
    end
    [ stree ] = add_lc_bank_node(stree,nidxes(i),rls(i),lls(i),cls(i),debug);
  end
  disp(['Over'])
end

%% Add LC Tank to a specific node
function [ stree ] = add_lc_bank_node(stree,nodeidx,rval,lval,cval,debug)

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
  newnode = stree.availnd;
  if (1 == debug)
    disp(['New Sub node created(Index=' num2str(newnode) ')'])
  end
  % Configure the new node
  % Initial the new node
  stree.bnode(newnode).btproc = 1;
  stree.bnode(newnode).x = stree.bnode(nodeidx).x;
  stree.bnode(newnode).y = stree.bnode(nodeidx).y;
  stree.bnode(newnode).cl = 0;
  % RLC info
  stree.bnode(newnode).r = 0;
  stree.bnode(newnode).l = 0;
  stree.bnode(newnode).c = 0;
  % sink info
  stree.bnode(newnode).issink = 0;
  stree.bnode(newnode).sidx = -1;
  % former node
  stree.bnode(newnode).numform = 1;
  stree.bnode(newnode).form(1).nidx = nodeidx;
  % Nexter node
  stree.bnode(newnode).numnext = stree.bnode(nodeidx).numnext;
  for icur = 1:stree.bnode(newnode).numnext
    stree.bnode(newnode).next(icur).nidx = stree.bnode(nodeidx).next(icur).nidx;
  end
  % Copy the left and right tree from nodeidx to new node
  stree.bnode(newnode).ltree.nidx = stree.bnode(nodeidx).ltree.nidx;
  stree.bnode(newnode).rtree.nidx = stree.bnode(nodeidx).rtree.nidx;
  
  % Configure the connections of subtree
  lidx = stree.bnode(newnode).ltree.nidx;
  ridx = stree.bnode(newnode).rtree.nidx;
  if (lidx ~= -1)
  stree.bnode(lidx).form(1).nidx = newnode;
  end
  if (ridx ~= -1)
  stree.bnode(ridx).form(1).nidx = newnode;
  end
  
  % Create a new node for LC bank
  lcbank = newnode + 1;
  if (1 == debug)
    disp(['LC bank node created(Index=' num2str(lcbank) ')'])
  end
  % Configure the lcbank node
  stree.bnode(lcbank).btproc = 1;
  stree.bnode(lcbank).x = stree.bnode(nodeidx).x;
  stree.bnode(lcbank).y = stree.bnode(nodeidx).y;
  stree.bnode(lcbank).cl = cval;
  % RLC info
  stree.bnode(lcbank).r = rval;
  stree.bnode(lcbank).l = lval;
  stree.bnode(lcbank).c = cval;
  % sink info
  stree.bnode(lcbank).issink = 1;
  stree.bnode(lcbank).sidx = 'lcbank';
  % former node
  stree.bnode(lcbank).numform = 1;
  stree.bnode(lcbank).form(1).nidx = nodeidx;
  % Nexter node
  stree.bnode(lcbank).numnext = 0;
  % Configure Left and Right Tree
  stree.bnode(lcbank).ltree.nidx = -1;
  stree.bnode(lcbank).rtree.nidx = -1;
  
  % Configure the old node
  stree.bnode(nodeidx).ltree.nidx = newnode;
  stree.bnode(nodeidx).rtree.nidx = lcbank;
  stree.bnode(nodeidx).numnext = 2;
  stree.bnode(nodeidx).next(1).nidx = newnode;
  stree.bnode(nodeidx).next(2).nidx = lcbank;

  % Update the stree availnd
  stree.availnd = lcbank + 1;
  
end

