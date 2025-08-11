# recc

[![C/C++ CI](https://github.com/rei0515reten/recc/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/rei0515reten/recc/actions/workflows/c-cpp.yml)

## make
```
make
```

## execute test
```
./test.sh
```

## debug(gdb)
1.Launch GDB with argument `1+2`(--args option)
```
gdb --args ./recc '1+2;'
```

2.If you want set break point to function main, please execute following command
```
break main
```

3.Run GDB
```
run
```

Exexute step
```
step (s)
```
```
next (n)
```

Display variable command
```
print <variable name>
```

Finish
```
quit
```


