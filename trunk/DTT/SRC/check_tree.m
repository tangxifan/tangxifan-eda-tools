
%% check_tree.m
% Check the tree : 
% 1. Check all the sinks are connected to source
% 2. Check whether there is any loop in the path from sinks to source 
% 
% Author : Xifan TANG
%          LSI, EPFL

%%
function [ stree ] = check_tree(stree,srcidx,debug)
 
  disp(['Checking paths from sinks to source and Loop...'])  

  % Find how many sink nodes there are
  numsink = 0;
  for ind = 1:(stree.availnd-1)  
    if (1 == stree.bnode(ind).issink)
      numsink = numsink + 1; 
    end  
  end
  sinklst = zeros(1,numsink);
  cur = 1;
  % Find all the sink nodes, store the index in list
  for ind = 1:(stree.availnd-1)  
    if (1 == stree.bnode(ind).issink)
      sinklst(cur) = ind;
      cur = cur + 1;
    end  
  end
  
  % Determine the path from sinks to source 
  for isink = 1:numsink
    cur = sinklst(isink);
    cursink = sinklst(isink);
    if (1 == debug)
      disp(['Checking ' num2str(isink) 'th Sink Node(Index=' num2str(cur) ')...'])
    end
    ipath = 1;
    checkrpt.sink(isink).path(ipath) = cur;
    % Find path
    while(1)
      if (0 == stree.bnode(cur).numform)
        if (srcidx ~= cur)
          disp(['Warning: Sink(Index=' num2str(cursink) ') does not connected to source!'])
        else
          if (1 == debug)
            disp(['Info: ' num2str(isink) 'th Sink(Index=' num2str(cursink) ') is connected to source!'])
            disp(['DEBUG: path[' num2str(checkrpt.sink(isink).path) ']'])
          end
        end
        break;
      else
        if (stree.bnode(cur).numform > 1)
          disp(['Error: Multiple driver for node(Index=' num2str(cur) ') found! Sink(Index=' num2str(cursink) ')']) 
          break;
        end
      end
    
      % Check connections
      conn = 0;
      formnd = stree.bnode(cur).form(1).nidx; 
      if (stree.bnode(formnd).ltree.nidx == cur) 
        conn = 1;
      end 
      if (stree.bnode(formnd).rtree.nidx == cur) 
        conn = 1;
      end 
      if (1 == conn)
        if (1 == debug)
          disp(['DEBUG: Node(Index=' num2str(formnd) ' Index=' num2str(cur) ') are connected!'])
        end
      else
        disp(['Error: Sink(Index=' num2str(cursink) ') Node(Index=' num2str(formnd) ' Index=' num2str(cur) ') are not connected!'])
      end
        
      cur = stree.bnode(cur).form(1).nidx; 
      % Check any repeated node in the path
      % which implies a loop existing
      for ip = 1:ipath
        if (cur == checkrpt.sink(isink).path(ip))
          disp(['Error: Loop Detected!, ip=' num2str(cur) ',Path=[' num2str(checkrpt.sink(isink).path) ']'])
          break;
        end
      end
      % Add current node to path
      ipath = ipath + 1;
      checkrpt.sink(isink).path(ipath) = cur;
    end
  end

  disp(['Over'])

end
