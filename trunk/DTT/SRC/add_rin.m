
%% ------------------------%
%  add_rin.m          %
%  Add Rin to source node  %
%  Author : Xifan TANG     %
%           LSI, EPFL      %
%--------------------------%

%% function add_rin
%  With given source node, add input resistance Rin
function [ stree newnode] = add_rin( stree,srcidx,rin,debug) 

  % Actually, two nodes are involved in this process
  % Assume, node 1 is nidx, and node 4 is inverter node. 
  % And node 2,3 are sub-nodes of node 1.
  %       Previous               After
  %           1(source)            4(new source)
  %          / \                   |
  %         2   3                  1(old source)
  %                               / \
  %                              2   3
  
  % 1st Step: Create New Node(Node 4)
  newnode = stree.availnd;
  % Update available node in stree
  stree.availnd = stree.availnd + 1;  
  % Configure new node 
  stree.bnode(newnode).btproc = 1;
  stree.bnode(newnode).x = stree.bnode(srcidx).x; 
  stree.bnode(newnode).y = stree.bnode(srcidx).y; 
  stree.bnode(newnode).cl = 0;
  % RLC info (In fact, it is inverter RC info)
  stree.bnode(newnode).r = rin;
  stree.bnode(newnode).l = 0;
  stree.bnode(newnode).c = 0;
  % Sink info
  stree.bnode(newnode).issink = 0;
  stree.bnode(newnode).sidx = -1;
  % Former node
  stree.bnode(newnode).numform = 0;
  stree.bnode(newnode).form(1).nidx = -1;  
  % Nexter node
  stree.bnode(newnode).numnext = 1;
  stree.bnode(newnode).next(1).nidx = srcidx;
  % Left&Rigth Tree
  stree.bnode(newnode).ltree.nidx = -1; 
  stree.bnode(newnode).rtree.nidx = srcidx; 
 
  % 2nd Step: Configure Current Node(Old Source)
  stree.bnode(srcidx).numform = 1;
  stree.bnode(srcidx).form(1).nidx = newnode;
 
  % Debug
  %if (1 == debug)
    disp(['Create New Source Node(Index=' num2str(newnode) ')'])
  %end 

end


