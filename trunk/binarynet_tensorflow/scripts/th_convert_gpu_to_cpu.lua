-- Convert a GPU t7 to a CPU t7
-- Author: Xifan Tang
require 'torch'
require 'nn'
require 'nngraph'
require 'cutorch'
require 'cunn'
require 'cudnn'
require 'newLayers.BinActiveZ'


cmd = torch.CmdLine()
cmd:text()
cmd:text('Convert a GPU checkpoint to CPU checkpoint.')
cmd:text()
cmd:text('Options')
cmd:argument('-model','GPU model checkpoint to convert')
cmd:text()

-- parse input params
local opt = cmd:parse(arg)

savefile = opt.model
savefile = savefile:gsub(".t7", "_cpu.t7")  -- append "cpu.t7" to filename
--local savefile = opt.model .. 'cpu.t7'  -- append "cpu.t7" to filename

print('Loading ' .. opt.model)
local checkpoint = torch.load(opt.model)
checkpoint = checkpoint:float()
torch.save(savefile, checkpoint)
print('saved ' .. savefile)

