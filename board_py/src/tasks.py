from invoke import task

@task
def buildBoardTest(c):
	build_path = "..\\..\\.\\board_cpp\\build"
	src_path = "..\\src" #relative to build path
	#-static-libstdc++
	c.run(f"cd {build_path} &&\
	   g++ -shared -c {src_path}\\*.cpp &&\
	   g++ -static -shared *.o -o board_cpp.dll \
	   -Wl,--export-all-symbols\
	   -Wl,--enable-auto-import"
	   )
	print("DLL built successfully")