.PHONY: ex001/output.h5

ex001/output.h5:
	PYTHONPATH=$(PWD):$(PYTHONPATH) python ex001/testme.py $@
