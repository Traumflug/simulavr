#
#  $Id$
#

EXTRA_DIST = pysimulavr.i setcfg.py README

CLEANFILES = pysimulavr_wrap.cpp pysimulavr_wrap.h pysimulavr.py pysimulavr.pyc _pysimulavr.so _pysimulavr.pyd

clean-local:
	-rm -rf build dist

pkginclude_HEADERS = \
    pysimulationmember.h

if SYS_MINGW
PYTHON_BDIST=bdist_wininst
else
PYTHON_BDIST=bdist
endif

python-build: _pysimulavr@PYTHON_MODULE_EXTENSION@
_pysimulavr@PYTHON_MODULE_EXTENSION@: pysimulavr.i $(pkginclude_HEADERS) @DOLLAR_SIGN@(addprefix ../,$(LIBSIM_HDRS))
if SYS_MINGW
	$(PYTHON) setcfg.py -c build_ext -o "library_dirs" -s "/mingw/lib"
	$(PYTHON) setcfg.py -c build_ext -o "libraries" -s "stdc++ ws2_32"
	$(PYTHON) setcfg.py -c build_ext -o "compiler" -s "mingw32"
endif
	EXT_SRCS="$(LIBSIM_SRCS)" $(PYTHON) setup.py build_ext $(PYTHON_BDIST)
	cp build/lib*/_pysimulavr@PYTHON_MODULE_EXTENSION@ .

all-local: python-build

# EOF
