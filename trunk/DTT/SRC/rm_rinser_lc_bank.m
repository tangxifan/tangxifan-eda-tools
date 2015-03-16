
%% rm_rinser_lc_bank.m
%  Remove LC Banks 
%  Author: Xifan TANG
%          LSI,EPFL
%
function [ stree ] = rm_rinser_lc_bank( stree,dirlst,indirlst,debug )

  % Remove LC banks with given node list
  [ stree ] = rm_lc_bank(stree,length(dirlst),dirlst,debug);
  disp(['Removing Indirect LC banks...'])
  for i = length(indirlst):-1:1
    if (debug == 1)
      disp(['Remove Indirect LC bank(NodeIndex=' num2str(nidxes(i)) ')'])
    end
    [ stree ] = rm_rinsert_lc_bank_node(stree,indirlst(i),debug);
  end
  disp(['Over'])
end

%% Remove LC Bank to a specific node
function [ stree ] = rm_rinsert_lc_bank_node(stree,nextidx,debug)
  
  nodeidx = stree.bnode(nextidx).form(1).nidx;
  former = stree.bnode(nodeidx).form(1).nidx;
  % Remove the LC Bank and the created node when adding the LC Bank 
  % Recover the connections
  if (stree.bnode(former).next(1).nidx == nodeidx)
    stree.bnode(former).next(1).nidx = nextidx;
  else
    stree.bnode(former).next(2).nidx = nextidx;
  end

  if (stree.bnode(former).ltree.nidx == nodeidx)
    stree.bnode(former).ltree.nidx = nextidx;
  else
    stree.bnode(former).rtree.nidx = nextidx;
  end
  stree.bnode(nextidx).form(1).nidx = former;
  % Recover the R,L,C
  stree.bnode(nextidx).r = stree.bnode(nextidx).r + stree.bnode(nodeidx).r;
  stree.bnode(nextidx).l = stree.bnode(nextidx).l + stree.bnode(nodeidx).l;
  stree.bnode(nextidx).c = stree.bnode(nextidx).c + stree.bnode(nodeidx).c;
  
  % Clear the information in new node
  % Clear the d,l,m,dtt
  cidx = nodeidx;
  % Clear the information in LC Bank node
  % Re-Initialize the Co-ordinator
  lcbidx = stree.bnode(cidx).rtree.nidx;
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

