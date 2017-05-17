#ifndef hep_hpc_hdf5_make_ntuple_hpp
#define hep_hpc_hdf5_make_ntuple_hpp
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::make_ntuple()
//
// Along with its supporting class, hep_hpc::hdf5::NtupleInitializer
// (described below), and hep_hpc::hdf5::make_column() /
// hep_hpc::hdf5::make_scalar_column() (see
// hep_hpc/hdf5/make_column.hpp) this function template is intended to
// simplify the construction of an hep_hpc::hdf5::Ntuple.
//
// The information required for the construction of an Ntuple can be
// considered in two categories: general information such as a filename,
// tablename, or translation mode; and column specific information as
// encoded in Column<...> object and the template arguments to Ntuple.
//
// To avoid having to specify template arguments to Ntuple, call
// make_ntuple as follows:
//
// hep_hpc::hdf5::make_ntuple({...}, <column-specifier>...);
//
// where {...} is a brace-enclosed-initializer-list of constructor
// arguments to NtupleInitializer (see below) specifying general Ntuple
// attributes, and <column-specifier> is a Column<<type>, <ndims>>
// specified either directly or by use of
// hep_hpc::hdf5::make_{,scalar_}column().
//
////////////////////////////////////////////////////////////////////////
// hep_hpc::hdf5::NtupleInitializer
//
// A helper class to allow the intuitive specification of general
// information (some of it optional) controlling the behavior of the
// Ntuple object to be created by hep_hpc::hdf5::make_ntuple().
//
// The constructors facilitate the specification of non-column-specific
// arguments as allowed by the various constructors of
// hep_hpc::hdf5::Ntuple (see hep_hpc/hdf5/Ntuple.hpp) and whose
// arguments (and defaults, where applicable) are as described
// therein. They can be summarized as:
//
// NtupleInitializer(hid_t file,
//                   std::string tablename,
//                   [TranslationMode mode,]
//                   bool overwriteContents = <default>,
//                   std::size_t bufsize = <default>);
//
// NtupleInitializer(std::string filename,
//                   std::string tablename,
//                   [TranslationMode mode,]
//                   std::size_t bufsize = <default>);
//
////////////////////////////////////////////////////////////////////////
#include "hep_hpc/hdf5/Ntuple.hpp"
#include "hep_hpc/hdf5/make_column.hpp"

namespace hep_hpc {
  namespace hdf5 {
    class NtupleInitializer;
    
    template <typename... Cols>
    Ntuple<Cols...>
    make_ntuple(NtupleInitializer ntInit,
                Cols && ... cols);
                
  } // Namespace hdf5.
} // Namespace hep_hpc.

class hep_hpc::hdf5::NtupleInitializer {
public:
  NtupleInitializer(hid_t file, std::string tablename);
  NtupleInitializer(hid_t file, std::string tablename,
                    bool overwriteContents);
  NtupleInitializer(hid_t file, std::string tablename,
                    bool overwriteContents,
                    std::size_t bufsize);
  NtupleInitializer(hid_t file, std::string tablename, TranslationMode mode);
  NtupleInitializer(hid_t file, std::string tablename,
                    TranslationMode mode,
                    bool overwriteContents);
  NtupleInitializer(hid_t file, std::string tablename,
                    TranslationMode mode,
                    bool overwriteContents,
                    std::size_t bufsize);
  NtupleInitializer(std::string filename, std::string tablename);
  NtupleInitializer(std::string filename, std::string tablename,
                    std::size_t bufsize);
  NtupleInitializer(std::string filename, std::string tablename,
                    TranslationMode mode);
  NtupleInitializer(std::string filename, std::string tablename,
                    TranslationMode mode,
                    std::size_t bufsize);
  
private:
  template <typename NT, typename... Cols>
  NT
  initializeNtuple(Cols && ... cols);

  // The only intended caller of initializeNtuple().
  template <typename... Cols>
  friend
  Ntuple<Cols...>
  make_ntuple(NtupleInitializer ntInit, Cols && ... cols);
  
  uint8_t cflag_{0};
  hid_t file_{-1};
  std::string filename_{};
  std::string tablename_{};
  TranslationMode mode_{TranslationMode::NONE};
  bool overwriteContents_{false};
  std::size_t bufsize_{0ull};
};

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename)
  :
  cflag_(0),
  file_(file),
  tablename_(std::move(tablename))
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename,
                  bool overwriteContents)
  :
  cflag_(1),
  file_(file),
  tablename_(std::move(tablename)),
  overwriteContents_(overwriteContents)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename,
                  bool overwriteContents,
                  std::size_t bufsize)
  :
  cflag_(2),
  file_(file),
  tablename_(std::move(tablename)),
  overwriteContents_(overwriteContents),
  bufsize_(bufsize)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename,
                  TranslationMode mode)
  :
  cflag_(3),
  file_(file),
  tablename_(std::move(tablename)),
  mode_(mode)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename,
                  TranslationMode mode,
                  bool overwriteContents)
  :
  cflag_(4),
  file_(file),
  tablename_(std::move(tablename)),
  mode_(mode),
  overwriteContents_(overwriteContents)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(hid_t file, std::string tablename,
                  TranslationMode mode,
                  bool overwriteContents,
                  std::size_t bufsize)
  :
  cflag_(5),
  file_(file),
  tablename_(std::move(tablename)),
  mode_(mode),
  overwriteContents_(overwriteContents),
  bufsize_(bufsize)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(std::string filename, std::string tablename)
  :
  cflag_(6),
  filename_(std::move(filename)),
  tablename_(std::move(tablename))
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(std::string filename, std::string tablename,
                  std::size_t bufsize)
  :
  cflag_(7),
  filename_(std::move(filename)),
  tablename_(std::move(tablename)),
  bufsize_(bufsize)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(std::string filename, std::string tablename,
                  TranslationMode mode)
  :
  cflag_(8),
  filename_(std::move(filename)),
  tablename_(std::move(tablename)),
  mode_(mode)
{
}

hep_hpc::hdf5::NtupleInitializer::
NtupleInitializer(std::string filename, std::string tablename,
                  TranslationMode mode,
                  std::size_t bufsize)
  :
  cflag_(9),
  filename_(std::move(filename)),
  tablename_(std::move(tablename)),
  mode_(mode),
  bufsize_(bufsize)
{
}

template <typename NT, typename... Cols>
NT
hep_hpc::hdf5::NtupleInitializer::
initializeNtuple(Cols && ... cols)
{
  // Switch statement is here to ensure that argument defaults are only
  // specified in one place, namely the Ntuple class template
  // definition.
  switch(cflag_) {
  case 0:
    return NT(file_, std::move(tablename_),
              typename NT::column_info_t { std::forward<Cols>(cols)... });
  case 1:
    return NT(file_, std::move(tablename_),
              typename NT::column_info_t { std::forward<Cols>(cols)... },
              overwriteContents_);
  case 2:
    return NT(file_, std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              overwriteContents_,
              bufsize_);
  case 3:
    return NT(file_, std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              mode_);
  case 4:
    return NT(file_, std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              mode_,
              overwriteContents_);
  case 5:
    return NT(file_, std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              mode_,
              overwriteContents_,
              bufsize_);
  case 6:
    return NT(std::move(filename_), std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... });
  case 7:
    return NT(std::move(filename_), std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              bufsize_);
  case 8:
    return NT(std::move(filename_), std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              mode_);
  case 9:
    return NT(std::move(filename_), std::move(tablename_),
              typename Ntuple<Cols...>::column_info_t { std::forward<Cols>(cols)... },
              mode_,
              bufsize_);
  default:
    throw std::logic_error("INTERNAL ERROR: in NtupleInitializer::initializeNtuple().");
  }
}

template <typename... Args>
hep_hpc::hdf5::Ntuple<Args...>
hep_hpc::hdf5::make_ntuple(NtupleInitializer ntInit,
                           Args && ... args)
{
  return ntInit.initializeNtuple<Ntuple<Args...>>(std::forward<Args>(args)...);
}

#endif /* hep_hpc_hdf5_make_ntuple_hpp */
