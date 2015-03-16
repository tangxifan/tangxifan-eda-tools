%% Determine the candidates which LC banks would be inserted
function [ direct_lst indirect_lst ] = get_candidate_nodes(stree,nodes,rthres,debug)

  numnode = size(nodes);
  direct_num = 0;
  indirect_num = 0;

  for ind = 1:numnode(1)
    nidx = nodes(ind,1);
    if (stree.bnode(nidx).accumr == rthres)
      direct_num = direct_num + 1;
    end
    if (stree.bnode(nidx).accumr > rthres)
      if (stree.bnode(nidx).numform > 0)
        formidx = stree.bnode(nidx).form(1).nidx;
        if (stree.bnode(formidx).accumr < rthres)
          indirect_num = indirect_num + 1;
        end 
      end
    end
  end

  direct_lst = zeros(1,direct_num);
  indirect_lst = zeros(1,indirect_num);
  curdir = 1;
  curindir = 1;

  for ind = 1:numnode(1)
    nidx = nodes(ind,1);
    if (stree.bnode(nidx).accumr == rthres)
      direct_lst(curdir) = nidx;
      curdir = curdir + 1;
    end
    if (stree.bnode(nidx).accumr > rthres)
      if (stree.bnode(nidx).numform > 0)
        formidx = stree.bnode(nidx).form(1).nidx;
        if (stree.bnode(formidx).accumr < rthres)
          indirect_lst(curindir) = nidx;
          curindir = curindir + 1;
        end 
      end
    end
  end

end
