
%% ------------------------%
%  add_inverter.m          %
%  Add Inverter to nodes   %
%  Author : Xifan TANG     %
%           LSI, EPFL      %
%--------------------------%

%% function add_inverter
%  With given node list, add inverters to these specific nodes
function [ stree ] = add_inverter( stree,n,nidxes,lci,lr,lco,debug) 

 % Add inverters one by one
 disp(['Add Inverters...'])
 for i = 1:n
   if (debug == 1)
     disp(['Add Inverter(Index=' num2str(nidxes(i)) ')']) 
   end
   [ stree ] = add_inv_node(stree,nidxes(i),lci(i),lr(i),lco(i),debug);
 end
 disp(['Over'])

end

%% function add_inv_node, add inverter to specific node
function [ stree ] = add_inv_node(stree,nidx,ci,r,co,debug)

  % Actually, four nodes are involved in this process
  % Assume, node 1 is nidx, and node 4 is inverter node. 
  % And node 2,3 are sub-nodes of node 1.
  %       Previous               After
  %           1                    1
  %          / \                   |
  %         2   3                  4(Inv)
  %                               / \
  %                              2   3
  
  % 1st Step: Create New Node(Node 4)
  newnode = stree.availnd;
  % Update available node in stree
  stree.availnd = stree.availnd + 1;  
  % Configure new node 
  stree.bnode(newnode).btproc = 1;
  stree.bnode(newnode).x = stree.bnode(nidx).x; 
  stree.bnode(newnode).y = stree.bnode(nidx).y; 
  stree.bnode(newnode).cl = 0;
  % RLC info (In fact, it is inverter RC info)
  stree.bnode(newnode).r = r;
  stree.bnode(newnode).l = 0;
  stree.bnode(newnode).c = co;
  % Sink info
  stree.bnode(newnode).issink = 0;
  stree.bnode(newnode).sidx = -1;
  % Former node
  stree.bnode(newnode).numform = 0;
  stree.bnode(newnode).form(1).nidx = nidx;  
  % Nexter node
  stree.bnode(newnode).numnext = stree.bnode(nidx).numnext;
  for icur = 1:stree.bnode(newnode).numnext
    stree.bnode(newnode).next(icur).nidx = stree.bnode(nidx).next(icur).nidx;
  end
  % Left&Rigth Tree
  stree.bnode(newnode).ltree.nidx = stree.bnode(nidx).ltree.nidx; 
  stree.bnode(newnode).rtree.nidx = stree.bnode(nidx).rtree.nidx; 
 
  % 2nd Step: Configure Current Node(Node 1)
  % Update CL and store ci_inv for remove purpose
  stree.bnode(nidx).ci_inv = ci;
  stree.bnode(nidx).c = stree.bnode(nidx).c + ci;
  % Nexter Node
  stree.bnode(nidx).numnext = 1;
  stree.bnode(nidx).next(1).nidx = newnode;  
  % Left&Right Tree 
  stree.bnode(nidx).ltree.nidx = -1;
  stree.bnode(nidx).rtree.nidx = -1;

  % 3rd step: Configure the Subtree Nodes(Node 2,3)
  lidx = stree.bnode(newnode).ltree.nidx;
  ridx = stree.bnode(newnode).rtree.nidx;
  if (lidx ~= -1)
    stree.bnode(lidx).form(1).nidx = newnode;
  end
  if (ridx ~= -1)
    stree.bnode(ridx).form(1).nidx = newnode;
  end
 
  % Debug
  if (1 == debug)
    disp(['Create Inverter Node(Index=' num2str(newnode) ')'])
  end 

end


