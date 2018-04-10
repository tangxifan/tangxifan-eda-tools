if filereadable("/etc/vim/vimrc.local")
  source /etc/vim/vimrc.local
endif

"Personal Settings by Xifan TANG
"-----------------
"Basic Settings
"-----------------
"
"Don't use incompatible keyboard
"set nocompatible
"detect file type
filetype on
"set history lines
set history=1000
"Use mouse at any buffer place
if has('mouse')
  set mouse=a
endif
set selection=exclusive
set selectmode=mouse,key
"------------------------
"User Interface Settings
"------------------------
"show line number
set nu!
"set background color
set background=dark
"------------
"Set format
"-----------
"Auto change line
set wrap
"set tab equals to 4 space
set tabstop=4
"----------------------------
"Programming Settings
"---------------------------
syntax on
"Set match mode, matching the parenthose
set showmatch
"------------------------
"Searching settings
"-----------------------
"Off the auto-highlight of matched words
set nohlsearch
