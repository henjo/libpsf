EXTRA_DIST = setup.py

if WITH_PYTHONBINDINGS

all-local:
	$(PYTHON) setup.py build

install-exec-local:
	$(PYTHON) setup.py install @PYTHON_INSTALL_PARAMS@

uninstall-local:
	rm -rf $(DESTDIR)$(libdir)/python*/*-packages/*libpsf*

clean-local:
	$(PYTHON) setup.py clean --all

endif