%% Insert LC Banks to specific node whose resistances meet the given threshold
function [ stree ] = full_rinsert_lc_bank(stree,rthres,dircands,dirlen,dirr,dirl,dirc,indircands,indirlen,indirr,indirl,indirc,debug)
  %% Insert the LC banks to candidates
  %% Direct Adds
  [ stree ] = add_lc_bank(stree,dirlen,dircands,dirr,dirl,dirc,debug);
  %% Indirect Adds
  [ stree ] = rinsert_lc_bank(stree,indirlen,indircands,rthres,indirr,indirl,indirc,debug);

end
