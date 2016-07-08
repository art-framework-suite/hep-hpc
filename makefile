.PHONY: ex001/output.h5

NRUNS=8
NSUBRUNS=2
NEVENTS=3

ex001/output.h5:
	@echo -n "Creating $@... "
	@rm -f $@
	@PYTHONPATH=$(PWD):$(PYTHONPATH) mpiexec -np 4 python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)
	@echo "done"
