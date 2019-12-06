all:
	rm -f cfg/cfg
ifeq ($(shell uname),Linux)
	cp -av bin/cfg-Linux cfg/cfg
	chmod +x cfg/cfg
endif
ifeq ($(shell uname),Darwin)
	cp -av bin/cfg-Darwin cfg/cfg
	chmod +x cfg/cfg
endif
