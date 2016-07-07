.PHONY: ex001/output.h5

ex001/output.h5:
	@echo -n "Creating $@... "
	@PYTHONPATH=$(PWD):$(PYTHONPATH) python ex001/testme.py $@
	@echo "done"
