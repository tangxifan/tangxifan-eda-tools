
%% new_build_tree.m
%  Build the Tree based on Benchmark
%  A NEW Version !
%  Author: Xifan TANG
%          LSI,EPFL
%

%%
function [ stree ] = new_build_tree(nodes,sinks,wire,wires,debug)
    
  % unit of Capaciance and Resistance
  % Please refer the the reference/explain_format.txt
  % where define the unit of resistance and capacitance.
  % unit_wire_R = Ohm/nm, unit_wire_C = fF/nm   
  % load_C = fF
  % All the coordinator are measured in nm.
  sinkcapunit = 1e-15;
  capunit = 1e-15;
  resunit = 1;

  % Offset between sink nodes and normal nodes
  sink_start_idx = 1;
  node_start_idx = 3;
  offsink = node_start_idx - sink_start_idx;

  % print statistics for debugging
  numnode = size(nodes);
  numsink = size(sinks);
  numwire = size(wires);
  numwiretype = size(wire);
  disp(['Number of node = ' num2str(numnode(1))])
  disp(['Number of sink = ' num2str(numsink(1))])
  disp(['Number of wire = ' num2str(numwire(1))])
  disp(['Number of wire type = ' num2str(numwiretype(1))])
  
  % Build a basic tree
  % Initialize all the basic nodes
  % Read all nodes
  for inode=1:numnode(1)
    % info of a normal node
    nidx = nodes(inode,1);
    stree.bnode(nidx).btproc = 0;
    %stree.bnode(inode).nidx = nodes(inode,1);
    stree.bnode(nidx).x = nodes(inode,2);
    stree.bnode(nidx).y = nodes(inode,3);
    % Initialize a sink node
    stree.bnode(nidx).cl = 0;
    stree.bnode(nidx).issink = 0;
    stree.bnode(nidx).sidx = -1; 
    % RLC info
    stree.bnode(nidx).r = 0;
    stree.bnode(nidx).l = 0;
    stree.bnode(nidx).c = 0;
    %Read all sinks
    if (((nidx - offsink) <= numsink(1))&&((nidx - offsink) > 0))
      stree.bnode(nidx).issink = 1;
      stree.bnode(nidx).cl = sinks(nidx-offsink,4)*sinkcapunit;
      stree.bnode(nidx).sidx = sinks(nidx-offsink,1);
      if ((sinks(nidx-offsink,2) ~= stree.bnode(nidx).x)||(sinks(nidx-offsink,3) ~= stree.bnode(nidx).y))
        disp(['Warning: Unmatch (X,Y) detected! Sink(X,Y)=(' num2str(sinks(nidx-offsink,2)) ',' num2str(sinks(nidx-offsink,3)) ') Node(X,Y)=(' num2str(stree.bnode(nidx).x) ',' num2str(stree.bnode(nidx).y) ')'])
      end
    end
    % info of connection
    stree.bnode(nidx).numnext = 0;
    stree.bnode(nidx).numform = 0;
    stree.bnode(nidx).form(1).nidx = -1;
    % Debug info
    stree.bnode(nidx).dtt = 0;
  end
  
  % Initial the wire length counter
  wirelen = 0;

  % Read all wires
  for iwr = 1:numwire(1)
    % make connections
    % Record src and des
    nsrc = wires(iwr,1);
    ndes = wires(iwr,2);
    % Find wire parameters
    wtype = wires(iwr,3);
    rper = 0;
    cper = 0;
    for itwr = 1:numwiretype(1)
      if (wire(itwr,1) == wtype)
        rper = wire(itwr,2);
        cper = wire(itwr,3);
        continue;
      end
    end
    % Cal wire length(Manhattan Distance)
    wl = abs(stree.bnode(nsrc).x-stree.bnode(ndes).x)+abs(stree.bnode(nsrc).y-stree.bnode(ndes).y);
    rw = rper * wl*resunit;
    cw = cper * wl*capunit;
    % Check whether all R or C is zero
    if (rw == 0)
      disp(['Warning: Zero Resistance of Wire (Index=' num2str(iwr) ')'])
    end
    if (cw == 0)
      disp(['Warning: Zero Capacitance of Wire (Index=' num2str(iwr) ')'])
    end
    % Update the stree basic nodes
    stree.bnode(nsrc).numnext = stree.bnode(nsrc).numnext + 1;
    stree.bnode(ndes).numform = stree.bnode(ndes).numform + 1;
    stree.bnode(nsrc).next(stree.bnode(nsrc).numnext).nidx = ndes;
    stree.bnode(ndes).form(stree.bnode(ndes).numform).nidx = nsrc;
    % Update wire resistance and capacitance
    % The wire resistance and capacitance are included in des node
    stree.bnode(ndes).r = rw;
    stree.bnode(ndes).l = 0;
    stree.bnode(ndes).c = cw + stree.bnode(ndes).cl;
    
    % Count the wire length
    wirelen = wirelen + wl;

  end

  disp(['Info: Total Wire Length is ' num2str(wirelen) 'nm'])
  
  % Check if there exists over two sub nodes 
  disp(['Checking The tree...'])
  for inode=1:numnode(1)
    % index of a normal node
    nidx = nodes(inode,1);
    % Check former
    if (stree.bnode(nidx).numform ~= 1)
      disp(['Warning: Node(Index=' num2str(nidx) ',Sink=' num2str(stree.bnode(nidx).issink) ') has ' num2str(stree.bnode(nidx).numform) 'form nodes'])
    end 
    if (1 == debug)
      % Check nexter
      if ((stree.bnode(nidx).numnext ~= 2) && (stree.bnode(nidx).issink == 0))
        disp(['Warning: Normal Node(Index=' num2str(nidx) ') has ' num2str(stree.bnode(nidx).numnext) 'next nodes'])
      end 
      if ((stree.bnode(nidx).numnext ~= 0) && (stree.bnode(nidx).issink == 1))
        disp(['Warning: Sink Node(Index=' num2str(nidx) ') has ' num2str(stree.bnode(nidx).numnext) 'next nodes'])
      end 
    end 
  end 
  
end

