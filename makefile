.PHONY: ex001/output.h5

ex001/output.h5:
	@echo -n "Creating $@... "
	@rm -f $@
	@PYTHONPATH=$(PWD):$(PYTHONPATH) python ex001/make_file.py $@
	@echo "done"
