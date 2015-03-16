
%% ----------------------------%
%   rm_rin.m                   %
%   remove input resistance    %
%   Author : Xifan TANG        %
%            LSI, EPFL         %
%------------------------------%

%% function rm_rin
%  With given node list, remove inverters from these nodes
function [ stree newsrc] = rm_rin(stree,srcidx,debug)
  
  %  Assume, Node 1 is new srcidx, Node 4 is srcidx.
  %  Node 2,3 are sub-nodes
  %      Previous         After
  %          4              1
  %          |             / \
  %          1            2   3
  %         / \       
  %        2   3

  newsrc = stree.bnode(srcidx).next(1).nidx;
  % 1st Step: Configure Node 1
  stree.bnode(newsrc).numform = 0;
  stree.bnode(newsrc).form = [];

  % 3rd Step: Initialize srcidx
  % Clear Basic Information
  stree.bnode(srcidx).btproc = 0;
  stree.bnode(srcidx).x = 0;
  stree.bnode(srcidx).y = 0;
  stree.bnode(srcidx).cl = 0;
  % Clear RLC info
  stree.bnode(srcidx).r = 0;
  stree.bnode(srcidx).l = 0;
  stree.bnode(srcidx).c = 0;
  % Clear Sink Info
  stree.bnode(srcidx).issink = 0; 
  stree.bnode(srcidx).sidx = -1; 
  % Former node
  stree.bnodes(srcidx).numform = 0;
  stree.bnode(srcidx).form = [];
  % Nexter node
  stree.bnode(srcidx).numnext = 0;
  stree.bnode(srcidx).next = [];
  % Left&Right Tree
  stree.bnode(srcidx).ltree = [];
  stree.bnode(srcidx).rtree = [];
  % Clear d,m,l,dtt
  stree.bnode(srcidx).d = [];
  stree.bnode(srcidx).n = [];
  stree.bnode(srcidx).m = [];
  stree.bnode(srcidx).dtt = [];
  stree.bnode(srcidx).dr = [];
  stree.bnode(srcidx).dl = [];
  stree.bnode(srcidx).mr = [];
  stree.bnode(srcidx).ml = [];
  stree.bnode(srcidx).impedr = 0;
  stree.bnode(srcidx).impedl = 0;
  stree.bnode(srcidx).imped = 0;
  
  % debug
  if (1 == debug)
    disp(['Remove Source Node(Index=' num2str(srcidx) ')']) 
  end

  if (stree.availnd == (srcidx+1))
    stree.availnd = stree.availnd - 1;
    disp(['Free Node(Availnd=' num2str(stree.availnd) ')'])
  end

end

