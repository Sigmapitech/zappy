if expand('%:f') !=# 'Doxyfile'
  finish
endif

" Comments
syntax match doxyComment "#.*$"
highlight link doxyComment Comment

" Keys (must come first, but match only the key part)
syntax match doxyKey "^\s*\zs[A-Z_]\+\ze\s*\(+=\|=\)"
highlight link doxyKey Keyword

" Operators
syntax match doxyOperator "\(+=\|=\)"
highlight link doxyOperator Operator

" Booleans
syntax match doxyBool "\<\(YES\|NO\)\>"
highlight link doxyBool Constant

" Strings in quotes
syntax region doxyString start=/"/ skip=/\\"/ end=/"/ keepend
highlight link doxyString String

" Patterns like *.c or *.h
syntax match doxyPattern "\*\.[ch]\(pp\)\?"
highlight link doxyPattern Type

let b:current_syntax = "doxygenconf"

