function [ stree,dircands,indircands] = report_rinsert_candidates(stree,nodes,srcidx,rthres,debug)
 %% Calculate the accumlated resistance
  disp(['Calculate the accumulated resistance...'])
  [ stree ] = rdfs_binary_tree(stree,srcidx,0,debug);
  disp(['Over'])

  %% Get the node list to insert the LC Banks
  disp(['Determine the candidates...'])
  [ dircands indircands] = get_candidate_nodes(stree,nodes,rthres,debug);
  disp(['Direct Candidates: ' num2str(dircands)])
  disp(['Indirect Candidates: ' num2str(indircands)])
  disp(['Over'])

end

