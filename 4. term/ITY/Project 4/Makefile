# Soubor: Makefile
# Reseni ITY-projekt4
# Datum: 7. 4. 2016
# Autor: Ondrej Vales, xvales03
# Fakulta: FIT VUTBR

all:
	latex proj4.tex
	bibtex proj4.aux
	latex proj4.tex
	latex proj4.tex
	dvips proj4.dvi
	ps2pdf -sPAPERSIZE=a4 proj4.ps

clean:
	rm -rf *.pdf *.dvi *.ps *.log *.aux *.bbl *.blg