.PHONY: test test-mpi
NRANKS=4
NRUNS_PER_RANK=3
NRUNS=$$(echo $$(( $(NRUNS_PER_RANK) * $(NRANKS) )))
NSUBRUNS=5
NEVENTS=30

test: ex001/output.h5
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+\s+Group$$" | wc -l) runs
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+\s+Group$$" | wc -l) subruns
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+/e_[[:digit:]]+\s+Group$$" | wc -l) events

ex001/output.h5: ex001/make_file.py makefile
	@printf "Creating $@... "
	@rm -f $@
	@PYTHONPATH=$(PWD):$(PYTHONPATH) python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)
	@echo "done"

test-mpi: ex001/output-mpi.h5
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+\s+Group$$" | wc -l) runs
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+\s+Group$$" | wc -l) subruns
	@echo $$(h5ls -lr $< | egrep "^/r_[[:digit:]]+/s_[[:digit:]]+/e_[[:digit:]]+\s+Group$$" | wc -l) events

ex001/output-mpi.h5: ex001/make_file.py makefile
	@echo "Creating $@... "
	@rm -f $@
	@PYTHONPATH=$(PWD):$(PYTHONPATH) USE_MPI=1 mpiexec -np $(NRANKS) python ex001/make_file.py $@ $(NRUNS) $(NSUBRUNS) $(NEVENTS)
	@echo "done"
