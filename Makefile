HTML:
	em++ -o out/main.js main.cpp -Os -I ./include -Wall lib/libraylib.a -s USE_GLFW=3 -s ASSERTIONS=1 -s SINGLE_FILE -g
WIN32:
	g++ -o out/main.exe main.cpp -Os -Wall -g ./lib/raylibdll.lib -I ./include