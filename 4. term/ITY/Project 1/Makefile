# Soubor: Makefile
# Reseni ITY-projekt1
# Datum: 29. 2. 2016
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

all: proj1.pdf

proj1.pdf: proj1.ps
	ps2pdf proj1.ps

proj1.ps: proj1.dvi proj1.tex
	dvips proj1.dvi

proj1.dvi: proj1.tex
	latex proj1.tex
	
clean:
	rm -f *.dvi *.aux *.log *.pdf *.ps