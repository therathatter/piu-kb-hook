all:
	g++ hook.cpp -o hook.so -std=c++11 -m32 -fPIC -shared -ldl -D_GNU_SOURCE
