
all: paper.pdf

.PHONY: clean
clean:
	rm -f *.bbl *.log paper.pdf *.log *.aux *.blg *.bcf *.slp

paper.pdf: paper.tex
	texi2pdf paper.tex
	bibtex paper
	texi2pdf paper.tex

