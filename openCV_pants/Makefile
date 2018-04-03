CC=g++
SOURCE=pants.cpp


OPENCV_FLAGS=$(shell pkg-config --cflags opencv;)
OPENCV_STATIC_PATH=$(shell for x in $$(ls /usr/local/lib/libopencv*.a; ls /usr/local/lib/libopencv*.a); do echo -n "$$x "; done)

BP=/usr/lib/x86_64-linux-gnu/
BASIC_LIBS=$(BP)libtiff.a $(BP)libcairo.a $(BP)libjpeg.a $(BP)libpixman-1.a   $(BP)liblzma.a $(BP)libjbig.a $(BP)libz.a  $(BP)libjasper.a  

TP=/usr/local/share/OpenCV/3rdparty/lib/
THIRD_PART_LIBS_PATH=$(TP)liblibwebp.a $(TP)libIlmImf.a $(TP)libittnotify.a $(TP)libippiw.a $(TP)libippicv.a

SHARED_LIBS=-ldl -lm -lpthread -lrt -ljasper -ljpeg -lpng -ltiff -lz -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpng12 -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0  -lrt -lavcodec -lavformat -lavutil -lswscale -ldc1394 -lv4l1

devmake: pants.cpp
	$(CC) pants.cpp `pkg-config --cflags --libs opencv` /usr/local/lib/libJsonBox.a
	
d: pants.cpp
	$(CC) -g pants.cpp `pkg-config --cflags --libs opencv` /usr/local/lib/libJsonBox.a

static:  $(SOURCE)
	g++ $(SOURCE) -o main  $(OPENCV_FLAGS) $(OPENCV_STATIC_PATH) /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a $(BASIC_LIBS) $(THIRD_PART_LIBS_PATH)  $(SHARED_LIBS)
	


