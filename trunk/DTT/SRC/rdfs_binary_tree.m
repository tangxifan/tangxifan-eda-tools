%% Calculate the accumulated resistance for each node
function [ stree ] = rdfs_binary_tree( stree,srcidx,rsum,debug)

  % Depth-First Search the Tree use recursive algorithm.
  if (stree.bnode(srcidx).rtree.nidx == -1)
    stree.bnode(srcidx).accumr = stree.bnode(srcidx).r+rsum;
  else
    rcuridx = stree.bnode(srcidx).rtree.nidx;
    stree.bnode(srcidx).accumr = stree.bnode(srcidx).r+rsum;
    [ stree ] = rdfs_binary_tree( stree,rcuridx,stree.bnode(srcidx).accumr,debug);
  end

  if (stree.bnode(srcidx).ltree.nidx == -1)
    stree.bnode(srcidx).accumr = stree.bnode(srcidx).r+rsum;
  else
    lcuridx = stree.bnode(srcidx).ltree.nidx;
    stree.bnode(srcidx).accumr = stree.bnode(srcidx).r+rsum;
    [ stree ] = rdfs_binary_tree( stree,lcuridx,stree.bnode(srcidx).accumr,debug);
  end

end
