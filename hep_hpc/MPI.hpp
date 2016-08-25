namespace hep_hpc {
  class MPI;
}

class hep_hpc::MPI {
public:
  MPI(int & argc, char **& argv);
  ~MPI();

  void abort(int exit_code = 1);
};
