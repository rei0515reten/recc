#!/bin/bash

assert(){
  expected="$1"
  input="$2"

  ./recc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 2 '(-3+5);'
assert 15 '-3*-5;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

#assert 1 'a = 1;'
#assert 5 'b = 1 + 4;'
#assert 6 'a = 1;b = 2 + 3;a + b;'
assert 10 'foo = 3;bar = 7;foo + bar;'
assert 15 'hoge = 3 + 5;hogehoge = 7;hoge + hogehoge;'
assert 4 'foo=7;bar=3;foo-bar;'
assert 15 'hoge=3+5;hogehoge=7;hoge+hogehoge;'
assert 8 'return 8;'
assert 6 'a = 4;b = 4;return a + b / 2;'
assert 1 'x = 2;if (x!=0) return 1;'
assert 0 'x = 0;if (x!=0) return 1;'
assert 0 'x = 0;if (x!=0) return 0; else return 1;'
#assert 3 'a = 0;if (a==0) b=3;'
#assert 3 'a = 0;if (a==0) b=3; else b=4;'

echo OK
