%%
%  Clock Tree Parser and
%  Direct Truncation of the Transfer Function(DTT)
%  on Clock Tree Structure
%
%  Author: Xifan TANG
%          LSI,EPFL
%
%%
clear all
clc

% Run the benckmark scrpit first
benchmark

% The index of source node
srcidx = 2; 

% Turn off debug by set it as zero
debug = 0;
freq = 1e9;
rin = 50;
rthres = 80;
rt = 0;
lt = 1e-9;
ct = 600e-12;

% Build the tree structure
clear H_L;
[ stree ] = new_build_tree(nodes,sinks,wire,wires,debug);

[ stree ] = build_binary_tree( stree,nodes,debug );
  
%check_tree(stree,srcidx,debug);

[ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);

[ stree srcidx] = add_rin(stree,srcidx,rin,debug);
[ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
[ stree srcidx] = rm_rin(stree,srcidx,debug);

%% Insert LC Banks With fixed resistance threshold
% DFS the tree and report the candidates nodes
[ stree,dircands,indircands] = report_rinsert_candidates(stree,nodes,srcidx,rthres,debug);
% Determine the R,L,C for LC banks
% For direct candidates
dirlen = length(dircands);
dirr = rt*ones(1,dirlen);
dirl = lt*ones(1,dirlen);
dirc = ct*ones(1,dirlen);
% For indirect candidates
indirlen = length(indircands);
indirr = rt*ones(1,indirlen);
indirl = lt*ones(1,indirlen);
indirc = ct*ones(1,indirlen);
% Insert these LC banks
[ stree ] = full_rinsert_lc_bank(stree,rthres,dircands,dirlen,dirr,dirl,dirc,indircands,indirlen,indirr,indirl,indirc,debug);
% Full DTT flow should be run again, to see the difference
[ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
% Remove these LC banks
[ stree ] = rm_rinser_lc_bank( stree,dircands,indircands,debug );
%% Add LC Banks
ndlen = 20;
node = zeros(1,ndlen);
r=zeros(1,ndlen);
c=60e-12*ones(1,ndlen);
% Generate Node List
ndidx = round(length(nodes)*rand(1,ndlen));
for i = 1:ndlen
  node(i) = nodes(ndidx(i),1);
end
% Test
for k = 1: 20
    L=k*1e-9;
    l=L*ones(1,ndlen);
    [ stree ] = add_lc_bank( stree,ndlen,node,r,l,c,0);
    [ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
    for i=1:length(stree.tfs)
      H_L2(i,k)=stree.tfs(i).hf;
    end
    [ stree ] = rm_lc_bank ( stree, ndlen, node, debug);
end
%% 20 LC Banks
node=[1019,1031,1124,1111,1025,1408,1475,1446,1552,1570,1577,530,1650,1643,593,1647,1696,655,1707,662];
ndlen = 20;
r=zeros(1,ndlen);
C=600e-12*ones(1,ndlen);
for k = 1: 20
    L=k*1e-9;
    l=L*ones(1,ndlen);
    [ stree ] = add_lc_bank( stree,20,node,r,l,C,0);
    [ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
    
    for i=1:length(stree.tfs)
      H_L20(i,k)=stree.tfs(i).hf;
    end
    [ stree ] = rm_lc_bank ( stree, 20, node, debug);
end
hold on
plot(H_L20','b')  
%% 15 LC BANKS
node=[400, 401, 711, 1037, 1038, 1169, 1269, 1329, 1445, 1734,  1748, 1873, 2102, 2104, 2128] ;
ndlen = 15;
r=zeros(1,ndlen);
C=600e-12*ones(1,ndlen);
for k = 1: 30
    L=k*4e-10;
    l=L*ones(1,ndlen);
    [ stree ] = add_lc_bank( stree,ndlen,node,r,l,C,0);
    [ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
    
    for i=1:length(stree.tfs)
      H_L15(i,k)=stree.tfs(i).hf;
    end
    Z_L15(k) = stree.zin;
    [ stree ] = rm_lc_bank ( stree, ndlen, node, debug);
end
hold on;
plot(H_L15')  
%% 8 LC Banks
node=[1019,1446,1757,2094, 2097, 2098, 2099,2101] ;
ndlen = 8;
r=zeros(1,ndlen);
C=60e-9*ones(1,ndlen);
for k = 1: 20
    L=k*4e-10;
    l=L*ones(1,ndlen);
    [ stree ] = add_lc_bank( stree,ndlen,node,r,l,C,0);
    [ stree ] = full_dtt(stree,srcidx,0,[],nodes,freq,debug);
    
    for i=1:length(stree.tfs)
      H_L8(i,k)=stree.tfs(i).hf;
    end
    Z_L8(k) = stree.zin;
    [ stree ] = rm_lc_bank ( stree, ndlen, node, debug);
end
hold on
plot(H_L8','b')  

