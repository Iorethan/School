# Soubor: Makefile
# Reseni ITY-projekt3
# Datum: 1. 4. 2016
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

all: proj3.pdf

proj3.pdf: proj3.ps
	ps2pdf proj3.ps

proj3.ps: proj3.dvi proj3.tex
	dvips -t a4 proj3.dvi

proj3.dvi: proj3.tex
	latex proj3.tex -sPAPERSIZE=a4
	latex proj3.tex -sPAPERSIZE=a4
	
clean:
	rm -f *.dvi *.aux *.log *.pdf *.ps