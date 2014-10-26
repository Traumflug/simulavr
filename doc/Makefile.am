#
# $Id$
#

SUBDIRS = web

# produce simulavr.pdf and simulavr.info
info_TEXINFOS = simulavr.texi

EXTRA_DIST = $(DX_CONFIG)

if HAVE_TEX
TEX_FORMATS = pdf
endif

if DX_COND_doc

## ------------------------------- ##
## Rules specific for HTML output. ##
## ------------------------------- ##
if DX_COND_html
DX_CLEAN_HTML = @DX_DOCDIR@/html
endif DX_COND_html

## ------------------------------ ##
## Rules specific for CHM output. ##
## ------------------------------ ##
if DX_COND_chm
DX_CLEAN_CHM = @DX_DOCDIR@/chm

if DX_COND_chi
DX_CLEAN_CHI = @DX_DOCDIR@/@PACKAGE@.chi
endif DX_COND_chi

endif DX_COND_chm

## ------------------------------ ##
## Rules specific for MAN output. ##
## ------------------------------ ##
if DX_COND_man
DX_CLEAN_MAN = @DX_DOCDIR@/man
endif DX_COND_man

## ------------------------------ ##
## Rules specific for RTF output. ##
## ------------------------------ ##
if DX_COND_rtf
DX_CLEAN_RTF = @DX_DOCDIR@/rtf
endif DX_COND_rtf

## ------------------------------ ##
## Rules specific for XML output. ##
## ------------------------------ ##
if DX_COND_xml
DX_CLEAN_XML = @DX_DOCDIR@/xml
endif DX_COND_xml

## ----------------------------- ##
## Rules specific for PS output. ##
## ----------------------------- ##
if DX_COND_ps
DX_CLEAN_PS = @DX_DOCDIR@/@PACKAGE@.ps
DX_PS_GOAL = doxygen-ps

doxygen-ps: @DX_DOCDIR@/@PACKAGE@.ps

@DX_DOCDIR@/@PACKAGE@.ps: @DX_DOCDIR@/@PACKAGE@.tag
	cd @DX_DOCDIR@/latex; \
	  rm -f *.aux *.toc *.idx *.ind *.ilg *.log *.out; \
	  $(DX_LATEX) refman.tex; \
	  $(MAKEINDEX_PATH) refman.idx; \
	  $(DX_LATEX) refman.tex; \
	  countdown=5; \
	  while $(DX_EGREP) 'Rerun (LaTeX|to get cross-references right)' \
	                    refman.log > /dev/null 2>&1 \
	        && test $$countdown -gt 0; do \
	    $(DX_LATEX) refman.tex; \
	    countdown=`expr $$countdown - 1`; \
	  done; \
	  $(DX_DVIPS) -o ../@PACKAGE@.ps refman.dvi
endif DX_COND_ps

## ------------------------------ ##
## Rules specific for PDF output. ##
## ------------------------------ ##

if DX_COND_pdf
DX_CLEAN_PDF = @DX_DOCDIR@/@PACKAGE@.pdf
DX_PDF_GOAL = doxygen-pdf

doxygen-pdf: @DX_DOCDIR@/@PACKAGE@.pdf

@DX_DOCDIR@/@PACKAGE@.pdf: @DX_DOCDIR@/@PACKAGE@.tag
	cd @DX_DOCDIR@/latex; \
	  rm -f *.aux *.toc *.idx *.ind *.ilg *.log *.out; \
	  $(DX_PDFLATEX) refman.tex; \
	  $(DX_MAKEINDEX) refman.idx; \
	  $(DX_PDFLATEX) refman.tex; \
	  countdown=5; \
	  while $(DX_EGREP) 'Rerun (LaTeX|to get cross-references right)' \
	                    refman.log > /dev/null 2>&1 \
	        && test $$countdown -gt 0; do \
	    $(DX_PDFLATEX) refman.tex; \
	    countdown=`expr $$countdown - 1`; \
	  done; \
	  mv refman.pdf ../@PACKAGE@.pdf
endif DX_COND_pdf

## ------------------------------------------------- ##
## Rules specific for LaTeX (shared for PS and PDF). ##
## ------------------------------------------------- ##
if DX_COND_latex
DX_CLEAN_LATEX = @DX_DOCDIR@/latex
endif DX_COND_latex

.PHONY: doxygen-run doxygen-doc install-doxygen $(DX_PS_GOAL) $(DX_PDF_GOAL)

.INTERMEDIATE: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

doxygen-run: @DX_DOCDIR@/@PACKAGE@.tag

doxygen-doc: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

@DX_DOCDIR@/@PACKAGE@.tag: $(DX_CONFIG) $(pkginclude_HEADERS)
	rm -rf @DX_DOCDIR@
	$(DX_ENV) $(DX_DOXYGEN) $(srcdir)/$(DX_CONFIG)

DX_CLEANFILES = doxygen.warnings.log -r @DX_DOCDIR@

install-doxygen: doxygen-doc
	for p in `find @DX_DOCDIR@ -type d`; do \
		install -d $(DESTDIR)$(docdir)/$$p; \
		find $$p -maxdepth 1 -a -type f -exec install -m 644 {} $(DESTDIR)$(docdir)/$$p \;; \
	done

endif DX_COND_doc

# Rules for generating doc with Sphinx
SPHINX_BUILDDIR = $(srcdir)/doc_build
SPHINXOPTS = -d $(SPHINX_BUILDDIR)/doctrees -D latex_paper_size=a4
SPHINXBUILD = @PYTHON@ -c "import sys;from pkg_resources import load_entry_point;sys.exit(load_entry_point('Sphinx', 'console_scripts', 'sphinx-build')())"
SPHINX_CLEANFILES = -r $(srcdir)/doc_build

sphinx-html:
if PYTHON_CMD_USE
if HAVE_PYMOD_sphinx
	$(SPHINXBUILD) -b html $(SPHINXOPTS) . $(SPHINX_BUILDDIR)/html
else
	@echo "  Can't build html documentation with Sphinx, python has no module sphinx!"
endif
else
	@echo "  Can't build html documentation with Sphinx, python command not found!"
endif

sphinx-pdf:
if PYTHON_CMD_USE
if HAVE_PYMOD_sphinx
if HAVE_PYMOD_rst2pdf
	$(SPHINXBUILD) -b pdf $(SPHINXOPTS) . $(SPHINX_BUILDDIR)/pdf
else
	@echo "  Can't build pdf documentation with Sphinx, python has no module rst2pdf!"
endif
else
	@echo "  Can't build pdf documentation with Sphinx, python has no module sphinx!"
endif
else
	@echo "  Can't build pdf documentation with Sphinx, python command not found!"
endif

sphinx-doc: sphinx-html sphinx-pdf

# Rules for generating web site with Sphinx
WEBSITE_BUILDDIR = $(srcdir)/web_build
WEBSITEOPTS = -E -c web -d $(WEBSITE_BUILDDIR)/doctrees
WEBSITE_CLEANFILES = -r $(srcdir)/web_build

web-html:
if PYTHON_CMD_USE
if HAVE_PYMOD_sphinx
	$(SPHINXBUILD) -b html $(WEBSITEOPTS) . $(WEBSITE_BUILDDIR)/html
else
	@echo "  Can't build web documentation with Sphinx, python has no module sphinx!"
endif
else
	@echo "  Can't build web documentation with Sphinx, python command not found!"
endif

CLEANFILES = stamp-vti $(DX_CLEANFILES) $(SPHINX_CLEANFILES) $(WEBSITE_CLEANFILES)

all-am: $(TEX_FORMATS) info


