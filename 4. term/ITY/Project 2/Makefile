# Soubor: Makefile
# Reseni ITY-projekt1
# Datum: 29. 2. 2016
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

all: proj2.pdf

proj2.pdf: proj2.ps
	ps2pdf proj2.ps
	ps2pdf proj2.ps

proj2.ps: proj2.dvi proj2.tex
	dvips -t a4 proj2.dvi
	dvips -t a4 proj2.dvi

proj2.dvi: proj2.tex
	latex proj2.tex
	latex proj2.tex
	
clean:
	rm -f *.dvi *.aux *.log *.pdf *.ps