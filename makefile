.PHONY: ex001/output.h5 test
NRANKS=1
NRUNS=$$(echo $$(( 3 * $(NRANKS) )))
NSUBRUNS=5

NEVENTS=3

test: ex001/output.h5
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+\s+Group$$" | wc -l) runs
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+\s+Group$$" | wc -l) subruns
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+/e_[[:digit:]]+\s+Group$$" | wc -l) events

ex001/output.h5:
	@printf "Creating $@... "
	@rm -f $@
	#@PYTHONPATH=$(PWD):$(PYTHONPATH) mpiexec -np $(NRANKS) python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)

	@PYTHONPATH=$(PWD):$(PYTHONPATH) python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)
	@echo "done"
