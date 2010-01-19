ROOT_DIR=..
ALL_TARGETS=pdf

include ../Makefile.mk

OUTPUT_PDF=uvudec-$(UVUDEC_VER).pdf

pdf: $(OUTPUT_PDF)
	@(true)

UVUDEC_HELP=uvudec_exe_help.tex
$(UVUDEC_HELP): $(UVUDEC_EXE)
	echo '\begin{verbatim}' >$(UVUDEC_HELP)
	$(UVUDEC_EXE) --help >>$(UVUDEC_HELP)
	echo '\end{verbatim}' >>$(UVUDEC_HELP)

$(OUTPUT_PDF): $(UVUDEC_HELP) *.tex
	
	echo |pdflatex --enable-pipes -halt-on-error uvudec.tex
	mv uvudec.pdf $(OUTPUT_PDF)
