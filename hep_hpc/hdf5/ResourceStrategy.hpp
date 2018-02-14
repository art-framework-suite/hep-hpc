#ifndef hep_hpc_hdf5_ResourceStrategy_hpp
#define hep_hpc_hdf5_ResourceStrategy_hpp
////////////////////////////////////////////////////////////////////////
// ResourceStrategy
//
// Simple enum to help higher-level resource managers (e.g. Dataset) to
// deal with already-allocated resources they should clean up, versus
// those they should not.
//
////////////////////////////////////////////////////////////////////////
namespace hep_hpc {
  namespace hdf5 {
    namespace detail {
      enum class ResourceStrategy : uint8_t { observer_tag, handle_tag };
    }
    using detail::ResourceStrategy;
  }
}

#endif /* hep_hpc_hdf5_ResourceStrategy_hpp */
