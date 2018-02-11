BIN=xvales03
QT_CONFIG=xvales03.pro
GENERATED_MAKEFILE=xvales03_makefile
QMAKE_MERLIN_PATH=/usr/local/share/Qt-5.5.1/5.5/gcc_64/bin/
QT_MERLIN_LIBS=/usr/local/share/Qt-5.5.1/5.5/gcc_64/lib/

all:
	PATH=${QMAKE_MERLIN_PATH}:${PATH} && qmake ${QT_CONFIG}
	sed -i '1iexport LD_LIBRARY_PATH=${QT_MERLIN_LIBS}:${LD_LIBRARY_PATH}' ${GENERATED_MAKEFILE}
	$(MAKE) -f ${GENERATED_MAKEFILE}

run:
	chmod +x ${BIN}
	./${BIN} 2> /dev/null

clean:
	rm -rf *.o moc_* ui_window.h ${GENERATED_MAKEFILE}

clean-bin: clean
	rm -rf ${BIN}

pack: clean
	zip -r xvales03.zip * 
	 
