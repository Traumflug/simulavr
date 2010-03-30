#
# $Id$
#

# Rules for generating web site with Sphinx
WEBSITE_BUILDDIR = $(srcdir)/web_build
WEBSITEOPTS = -d $(WEBSITE_BUILDDIR)/doctrees -D latex_paper_size=a4
WEBSITEBUILD = @PYTHON@ -c "import sys;from pkg_resources import load_entry_point;sys.exit(load_entry_point('Sphinx', 'console_scripts', 'sphinx-build')())"
WEBSITE_CLEANFILES = -r $(srcdir)/web_build

web-html:
if PYTHON_USE
	$(WEBSITEBUILD) -b html $(WEBSITEOPTS) . $(WEBSITE_BUILDDIR)/html
else
	@echo "  Can't build web site with Sphinx, python not configured!"
endif

CLEANFILES = $(WEBSITE_CLEANFILES)

