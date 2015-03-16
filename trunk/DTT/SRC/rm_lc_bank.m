
%% rm_lc_bank.m
%  Remove LC Banks 
%  Author: Xifan TANG
%          LSI,EPFL
%
function [ stree ] = rm_lc_bank( stree,n,nidxes,debug )

  % Remove LC banks with given node list
  disp(['Removing Direct LC banks...'])
  for i = n:-1:1
    if (debug == 1)
      disp(['Remove Direct LC bank(NodeIndex=' num2str(nidxes(i)) ')'])
    end
    [ stree ] = rm_lc_bank_node(stree,nidxes(i),debug);
  end
  disp(['Over'])
end

%% Remove LC Bank to a specific node
function [ stree ] = rm_lc_bank_node(stree,nodeidx,debug)
  
  % Remove the LC Bank and the created node when adding the LC Bank 
  % Clear the d,l,m,dtt
  stree.bnode(nodeidx).d = [];
  stree.bnode(nodeidx).n = [];
  stree.bnode(nodeidx).m = [];
  stree.bnode(nodeidx).dtt = 0;
  % Disjoint the subnodes at nodeidx
  % And New node locates at left tree
  cidx = stree.bnode(nodeidx).ltree.nidx;
  % When the LC bank always locates at right tree
  lcbidx = stree.bnode(nodeidx).rtree.nidx;
  % Connect the node to the subtree of new node
  stree.bnode(nodeidx).numnext = stree.bnode(cidx).numnext;
  for icur = 1:stree.bnode(cidx).numnext
    stree.bnode(nodeidx).next(icur).nidx = stree.bnode(cidx).next(icur).nidx;  
  end
  % Configure the left tree and right tree
  stree.bnode(nodeidx).ltree.nidx = stree.bnode(cidx).ltree.nidx;
  stree.bnode(nodeidx).rtree.nidx = stree.bnode(cidx).rtree.nidx;
  
  % Configure the former of subtree
  lidx = stree.bnode(nodeidx).ltree.nidx;
  ridx = stree.bnode(nodeidx).rtree.nidx;
  if (lidx ~= -1)
    stree.bnode(lidx).form(1).nidx = nodeidx;
  end
  if (ridx ~= -1)
    stree.bnode(ridx).form(1).nidx = nodeidx;
  end
  
  % Clear the information in new node
  % Re-Initialize the Co-ordinator
  stree.bnode(cidx).x = 0;
  stree.bnode(cidx).y = 0;
  stree.bnode(cidx).cl = 0;
  % Re-Initialize the RLC
  stree.bnode(cidx).r = 0;
  stree.bnode(cidx).l = 0;
  stree.bnode(cidx).c = 0;
  % Re-Initialize Sink Attributes
  stree.bnode(cidx).issink = 0;
  stree.bnode(cidx).sidx = -1;
  % ReInitialize the connections
  stree.bnode(cidx).numform = 0;
  stree.bnode(cidx).form = [];
  stree.bnode(cidx).numnext = 0;
  stree.bnode(cidx).next = [];
  stree.bnode(cidx).ltree = [];
  stree.bnode(cidx).rtree = [];
  % Clear the d,l,m,dtt
  stree.bnode(cidx).d = [];
  stree.bnode(cidx).n = [];
  stree.bnode(cidx).m = [];
  stree.bnode(cidx).dtt = 0;

  % Clear the information in LC Bank node
  % Re-Initialize the Co-ordinator
  stree.bnode(lcbidx).x = 0;
  stree.bnode(lcbidx).y = 0;
  stree.bnode(lcbidx).cl = 0;
  % Re-Initialize the RLC
  stree.bnode(lcbidx).r = 0;
  stree.bnode(lcbidx).l = 0;
  stree.bnode(lcbidx).c = 0;
  % Re-Initialize Sink Attributes
  stree.bnode(lcbidx).issink = 0;
  stree.bnode(lcbidx).sidx = -1;
  % ReInitialize the connections
  stree.bnode(lcbidx).numform = 0;
  stree.bnode(lcbidx).form = [];
  stree.bnode(lcbidx).numnext = 0;
  stree.bnode(lcbidx).next = [];
  stree.bnode(lcbidx).ltree = [];
  stree.bnode(lcbidx).rtree = [];
  % Clear the d,l,m,dtt
  stree.bnode(lcbidx).d = [];
  stree.bnode(lcbidx).n = [];
  stree.bnode(lcbidx).m = [];
  stree.bnode(lcbidx).dtt = 0;
 
  % Release the new node
  stree.bnode(cidx).btproc = 0;
  % Release the LC bank
  stree.bnode(lcbidx).btproc = 0;
  
  if (1 == debug)
    disp(['Remove LC bank(Index=' num2str(lcbidx) ')'])
    disp(['Remove Node(Index=' num2str(cidx) ')'])
  end
  
  if (stree.availnd == (lcbidx + 1))
    stree.availnd = stree.availnd - 1;
    if (1 == debug)
      disp(['Free Node(Availnd=' num2str(stree.availnd) ')'])
    end
  end
  if (stree.availnd == (cidx + 1))
    stree.availnd = stree.availnd - 1;
    if (1 == debug)
      disp(['Free Node(Availnd=' num2str(stree.availnd) ')'])
    end
  end
  
  
end

