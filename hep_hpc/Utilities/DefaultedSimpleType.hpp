#ifndef hep_hpc_Utilities_DefaultedSimpleType_hpp
#define hep_hpc_Utilities_DefaultedSimpleType_hpp

namespace hep_hpc {
  namespace detail {
    template <typename T, T DEF = {}>
    class DefaultedSimpleType;
  }
}

template <typename T, T DEF>
class hep_hpc::detail::DefaultedSimpleType {
public:
  DefaultedSimpleType() : item_{DEF} { }
  DefaultedSimpleType(T t) : item_{t} { }

  operator T & () { return item_; }
  operator T const & () const { return item_; }

private:
  T item_;
};

#endif /* hep_hpc_Utilities_DefaultedSimpleType_hpp */
