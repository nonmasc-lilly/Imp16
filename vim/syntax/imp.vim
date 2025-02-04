syn keyword impKeyword          segment segend label end bind int halt load
syn keyword impRegister         low high seg accumulator pointer counter general source destination stack base extra data
syn match   impSegOpt           "\.\(at\|sizeof\|prefix\|suffix\|entry\|prepend_entry\|id\)"
syn match   impName             "[a-zA-Z_][a-zA-Z0-9_]*"
syn match   impNumber           "[0-9]\+\|\$[0-9a-fA-F]\+"
syn match   impPunctuator       ":=\|:"
syn region  impComment          start="\\" end="\\"

hi def link impKeyword          Keyword
hi def link impSegOpt           PreProc
hi def link impRegister         Type
hi def link impNumber           Constant
hi def link impName             Identifier
hi def link impPunctuator       Operator
hi def link impComment          Comment
