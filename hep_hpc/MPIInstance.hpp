#ifndef hep_hpc_MPIInstance_hpp
#define hep_hpc_MPIInstance_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::MPIInstance
//
// Simple RAII class to handle MPI operations and safe shutdown at the
// end of the job.
//
// Instantiate an hep_hpc::MPIInstance on the stack in main() before starting
// MPI operations. Be sure to call hep_hpc::MPIInstance::abort() in the event
// of an exception bubbling up to main() in order to ensure safe
// shutdown. At normal program exit, MPI_Finalize() will be called as
// program execution leaves main().
//
// By default, MPI_COMM_WORLD or a newly-created communicator is
// configured to abort execution in the event of any MPI
// error. Appending the optional argument
// MPIInstance::world_errors_return_tag will default MPI_COMM_WORLD
// errors to return to allow the error to be handled locally (perhaps
// with hep_hpc::throwOnMPIError()). This error handler setting will be
// propagated to new communicators based on MPI_COMM_WORLD.
////////////////////////////////////////////////////////////////////////

namespace hep_hpc {
  class MPIInstance;
}

class hep_hpc::MPIInstance {
private:
  class WORLD_ERRORS_RETURN_t { };
public:
  MPIInstance(int & argc, char **& argv);
  MPIInstance(int & argc, char **& argv, WORLD_ERRORS_RETURN_t);
  ~MPIInstance();

  static constexpr WORLD_ERRORS_RETURN_t world_errors_return_tag { };

  // Call this abort() function to trigger safe shutdown of MPI
  // operations (short-cut for calling MPI_Abort()).
  static void abort(int exit_code = 1);
};

#endif /* hep_hpc_MPIInstance_hpp */
