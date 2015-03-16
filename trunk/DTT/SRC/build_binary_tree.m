%%
%  Build Binary Tree based on Benchmark
%
%  Author: Xifan TANG
%          LSI,EPFL
%

%% A few new nodes are created in this function for binary tree generation
%  All the new nodes are stored in structure stree.
function [ stree ] = build_binary_tree( stree,nodes,debug )
   
  % print statistics for debugging
  numnode = size(nodes);
 
  % Get the available index in the stree.bnode list
  maxidx = -1;
  for inode=1:numnode(1)
    % info of a normal node
    nidx = nodes(inode,1);
    if ((maxidx == -1)||(maxidx < nidx))
      maxidx = nidx;
    end
  end
  availnd = maxidx + 1;
  stree.availnd = availnd;
  disp(['Before Binary Tree Organzation Availnd in stree = ' num2str(stree.availnd)])

  % Read all nodes
  for inode=1:numnode(1)

    nidx = nodes(inode,1);
    % Find the flag of node
    if  (stree.bnode(nidx).btproc == 1)
      continue;
    end
    % Deal with normal node
    if (stree.bnode(nidx).issink == 0)
      % In case some node has no subtree
      if (stree.bnode(nidx).numnext == 0)
        if (debug == 1)
          disp(['Action: Normal Node(Index=' num2str(nidx) 'NextNum=' num2str(stree.bnode(nidx).numnext) ') NULL'])
        end
        % Left tree
        stree.bnode(nidx).ltree.nidx = -1;
        % Right tree
        stree.bnode(nidx).rtree.nidx = -1;
      elseif (stree.bnode(nidx).numnext == 1)
        if (debug == 1)
          disp(['Action: Normal Node(Index=' num2str(nidx) 'NextNum=' num2str(stree.bnode(nidx).numnext) ') Left Tree'])
        end
        % Left tree
        stree.bnode(nidx).ltree.nidx = stree.bnode(nidx).next(1).nidx;
        % Right tree
        stree.bnode(nidx).rtree.nidx = -1;
        % Update stree
        stree.bnode(nidx).numnext = 2;
        stree.bnode(nidx).next(2).nidx = -1;
      else
        curidx = nidx;
        while(1)
          cur = stree.bnode(curidx).numnext;
          if (cur == 2)
             if (debug == 1)
               disp(['Action: Normal Node(Index=' num2str(curidx) 'NextNum=' num2str(stree.bnode(curidx).numnext) ') Left&Right Tree!'])
             end
             % Process the left tree
             stree.bnode(curidx).ltree.nidx = stree.bnode(curidx).next(1).nidx;
             stree.bnode(curidx).rtree.nidx = stree.bnode(curidx).next(2).nidx;
             break;
          else
            % Process the left tree
            stree.bnode(curidx).ltree.nidx = stree.bnode(curidx).next(cur).nidx;
            if (debug == 1)
              disp(['Action: Normal Node(Index=' num2str(curidx) ' NextNum=' num2str(stree.bnode(curidx).numnext) ' Cur=' num2str(cur) ') Add extra Node(Index=' num2str(availnd) ')!'])
            end
            % Create a new node from availnd
            cur = cur - 1;
            % Assign basic attributes
            stree.bnode(availnd).btproc = 0;
            stree.bnode(availnd).x = stree.bnode(curidx).x;
            stree.bnode(availnd).y = stree.bnode(curidx).y;
            stree.bnode(availnd).r = 0;
            stree.bnode(availnd).l = 0;
            stree.bnode(availnd).c = 0;
            % Initialize a sink node
            stree.bnode(availnd).cl = 0;
            stree.bnode(availnd).issink = 0;
            stree.bnode(availnd).sidx = -1; 
            % Assign form node
            stree.bnode(availnd).numform = 1;
            stree.bnode(availnd).form(1).nidx = curidx; % the former index cannot be trusted!
            % Assign Debug
            stree.bnode(availnd).dtt = 0;
            % Assign next node
            stree.bnode(availnd).numnext = cur;
            for icur = 1:cur
              % Copy the connections
              stree.bnode(availnd).next(icur).nidx = stree.bnode(curidx).next(icur).nidx;
              % Assign former connections
              stree.bnode(stree.bnode(curidx).next(icur).nidx).form(1).nidx = availnd;
            end
            % Configure the nidx (Current node)
            stree.bnode(curidx).rtree.nidx = availnd;
            % Delete the other next nodes
            stree.bnode(curidx).numnext = 2;
            % New Node
            stree.bnode(curidx).next(1).nidx = availnd;
            % Exist node (Move its position)
            stree.bnode(curidx).next(2).nidx = stree.bnode(curidx).next(cur + 1).nidx;
            % Update curidx
            stree.bnode(availnd).btproc = 1;
            % Update Available nodes
            curidx = availnd;
            availnd = availnd + 1;
          end
        end
      end
      
    % Deal with sink node
    else
      % Left tree
      stree.bnode(nidx).ltree.nidx = -1;
     % Right tree
      stree.bnode(nidx).rtree.nidx = -1;
    end
    stree.bnode(nidx).btproc = 1;
  end
    
  % Update Available nodes in struct
  stree.availnd = availnd;
  disp(['After Binary Tree Organzation Availnd in stree = ' num2str(stree.availnd)])
  
  % Check if there exists over two sub nodes 
  disp(['Checking The tree again...'])
  for inode=1:numnode(1)
    % index of a normal node
    nidx = nodes(inode,1);
    % Check former
    if (stree.bnode(nidx).numform ~= 1)
      disp(['Warning: Node(Index=' num2str(nidx) ',Sink=' num2str(stree.bnode(nidx).issink) ') has ' num2str(stree.bnode(nidx).numform) 'form nodes'])
    end 
    % Check nexter
    if ((stree.bnode(nidx).numnext ~= 2) && (stree.bnode(nidx).issink == 0))
      disp(['Warning: Normal Node(Index=' num2str(nidx) ') has ' num2str(stree.bnode(nidx).numnext) 'next nodes'])
    end 
    if ((stree.bnode(nidx).numnext ~= 0) && (stree.bnode(nidx).issink == 1))
      disp(['Warning: Sink Node(Index=' num2str(nidx) ') has ' num2str(stree.bnode(nidx).numnext) 'next nodes'])
    end 
  end 
end

