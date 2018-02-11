BIN=antispam
SRC=antispam.py
RUN=antispam.run
DOC=doc.pdf

all:
	cp ${RUN} ${BIN}
	chmod +x ${BIN}
	pip3 install --user beautifulsoup4

clean:
	rm -rf ${BIN}

pack: clean
	zip xvales03.zip ${SRC} ${RUN} ${DOC} Makefile
