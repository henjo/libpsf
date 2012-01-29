EXTRA_DIST = setup.py

if WITH_PYTHONBINDINGS

all-local:
	$(PYTHON) setup.py build_ext --include-dirs=../../include:@BOOST_ROOT_PATH@/include --library-dirs=../../src/.libs:@BOOST_ROOT_PATH@/lib 

install-exec-local:
	$(PYTHON) setup.py install --prefix=$(prefix) --root=$(DESTDIR)

uninstall-local:
	rm -rf $(DESTDIR)$(libdir)/python*/*-packages/*libpsf*

clean-local:
	$(PYTHON) setup.py clean --all

endif