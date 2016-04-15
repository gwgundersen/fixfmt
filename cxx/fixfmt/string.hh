#pragma once

#include <cassert>
#include <string>

#include "fixfmt/text.hh"

//------------------------------------------------------------------------------

namespace fixfmt {

using std::string;

class String
{
public:

  String(
    int     size,
    string  ellipsis="\u2026",
    string  pad=" ",
    double  position=1.0,
    bool    pad_left=false);

  size_t        get_width()     const { return size_; }

  int           get_size()      const { return size_; }
  string const& get_ellipsis()  const { return ellipsis_; }
  string const& get_pad()       const { return pad_; }
  double        get_position()  const { return position_; }
  bool          get_pad_left()  const { return pad_left_; }

  string operator()(string const& str) const;

private:

  int       const size_;
  string    const ellipsis_;
  string    const pad_;
  double    const position_;
  bool      const pad_left_;

};


inline 
String::String(
  int     const size,
  string        ellipsis,
  string        pad,
  double  const position,
  bool    const pad_left)
: size_(size),
  ellipsis_(std::move(ellipsis)),
  pad_(std::move(pad)),
  position_(position),
  pad_left_(pad_left)
{
  assert(ellipsis_.length() <= (size_t) size_);
  assert(0 <= position_ && position_ <= 1);
}


inline string 
String::operator()(
  string const& str) 
  const
{
  return palide(str, size_, ellipsis_, pad_, position_, pad_left_);
}


}  // namespace fixfmt

