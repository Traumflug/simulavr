#
#  $Id$
#

EXTRA_DIST = pysimulavr.i setcfg.py README

CLEANFILES = pysimulavr_wrap.cpp pysimulavr_wrap.h pysimulavr.py pysimulavr.pyc

clean-local:
	-rm -rf build dist .module-built _pysimulavr*.so _pysimulavr*.pyd

pkginclude_HEADERS = \
    pysimulationmember.h

if HAVE_PYMOD_wheel
PYTHON_BDIST=bdist_wheel
else
if HAVE_PYMOD_setuptools
PYTHON_BDIST=bdist_egg
else
if SYS_MINGW
PYTHON_BDIST=bdist_wininst
else
PYTHON_BDIST=bdist
endif
endif
endif

python-build: .module-built
.module-built: pysimulavr.i $(pkginclude_HEADERS) @DOLLAR_SIGN@(addprefix ../,$(LIBSIM_HDRS))
if SYS_MINGW
	$(PYTHON) setcfg.py -c build_ext -o "library_dirs" -s "/mingw/lib"
	$(PYTHON) setcfg.py -c build_ext -o "libraries" -s "stdc++ ws2_32"
	$(PYTHON) setcfg.py -c build_ext -o "compiler" -s "mingw32"
endif
	EXT_SRCS="$(LIBSIM_SRCS)" $(PYTHON) setup.py build_ext $(PYTHON_BDIST)
	cp build/lib*/_pysimulavr*@PYTHON_MODULE_EXTENSION@ .
	touch .module-built

all-local: python-build

.PHONY: python-build

# EOF
