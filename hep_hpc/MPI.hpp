////////////////////////////////////////////////////////////////////////
// hep_hpc::MPI
//
// Simple RAII class to handle MPI operations and safe shutdown at the
// end of the job.
//
// Instantiate an hep_hpc::MPI on the stack in main() before starting
// MPI operations. Be sure to call hep_hpc::MPI::abort() in the event
// of an exception bubbling up to main() in order to ensure safe
// shutdown. At normal program exit, MPI_Finalize() will be called as
// program execution leaves main().
////////////////////////////////////////////////////////////////////////
namespace hep_hpc {
  class MPI;
}

class hep_hpc::MPI {
public:
  MPI(int & argc, char **& argv);
  ~MPI();

  // Call this abort() function to trigger safe shutdown of MPI
  // operations.
  void abort(int exit_code = 1);
};
