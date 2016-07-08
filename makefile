.PHONY: ex001/output.h5 test
NRANKS=2
NRUNS=$$(echo $$(( 2 * $(NRANKS) )))
NSUBRUNS=2
NEVENTS=3

test: ex001/output.h5
	@echo $$(h5ls -lr $< | egrep "^/r_\d+\s+Group$$" | wc -l) runs
	@echo $$(h5ls -lr $< | egrep "^/r_\d+/s_\d+\s+Group$$" | wc -l) subruns
	@echo $$(h5ls -lr $< | egrep "^/r_\d+/s_\d+/e_\d+\s+Group$$" | wc -l) events

ex001/output.h5:
	@echo -n "Creating $@... "
	@rm -f $@
	@PYTHONPATH=$(PWD):$(PYTHONPATH) mpiexec -np $(NRANKS) python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)
	@echo "done"
