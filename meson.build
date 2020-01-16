project('afbidl', 'c')

jsondep = dependency('json-c')

src = ['getref.c', 'exprefs.c', 'json2c.c']

executable('exprefs', 'main-exprefs.c', sources : src, dependencies : jsondep)
executable('genskel', 'main-genskel.c', sources : src, dependencies : jsondep)
executable('json2c' , 'main-json2c.c' , sources : src, dependencies : jsondep)
