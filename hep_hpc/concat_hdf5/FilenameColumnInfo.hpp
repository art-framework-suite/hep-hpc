#ifndef hep_hpc_concat_hdf5_FilenameColumnInfo_hpp
#define hep_hpc_concat_hdf5_FilenameColumnInfo_hpp

#include <regex>
#include <string>
#include <vector>

namespace hep_hpc {
  class FilenameColumnInfo;
}

class hep_hpc::FilenameColumnInfo {
public:
  FilenameColumnInfo() = default;
  explicit FilenameColumnInfo(std::string column_name)
    : column_name_(std::move(column_name))
    {
    }
  FilenameColumnInfo(std::string column_name,
                     std::regex re,
                     std::string replacement)
    : column_name_(std::move(column_name))
    , re_(std::move(re))
    , replacement_(std::move(replacement))
    {
    }

  bool has_regex() const { return !replacement_.empty(); }
  bool has_group_regex() const { return (group_regexes_.size() > 0); }

  std::string const & column_name() const { return column_name_; }
  std::regex const & regex() const { return re_; }
  std::string const & replacement() const { return replacement_; }
  std::vector<std::regex> const & group_regexes() { return group_regexes_; }
  void set_group_regexes(std::vector<std::regex> gre) { group_regexes_ = std::move(gre); }

private:
  std::string column_name_;
  std::regex re_;
  std::string replacement_;
  std::vector<std::regex> group_regexes_;
};

#endif /* hep_hpc_concat_hdf5_FilenameColumnInfo_hpp */
