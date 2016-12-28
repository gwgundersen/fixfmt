#include <cassert>
#include <cmath>
#include <limits>
#include <string>

#include "fixfmt/text.hh"
#include "py.hh"

using namespace py;
using std::string;

//------------------------------------------------------------------------------

// FIXME: Add docstrings.

namespace {

template<typename TYPE>
ref<Object> analyze_float(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {"buf", "max_precision", nullptr};
  PyObject* array_obj;
  int max_precision;
  Arg::ParseTupleAndKeywords(
    args, kw_args, "Oi", arg_names, &array_obj, &max_precision);

  BufferRef buffer(array_obj, PyBUF_ND);
  if (buffer->ndim != 1)
    throw TypeError("not a one-dimensional array");
  if (buffer->itemsize != sizeof(TYPE))
    throw TypeError("wrong itemsize");
  TYPE const* const array = (TYPE const* const) buffer->buf;
  size_t const length = buffer->shape[0];

  bool has_nan = false;
  bool has_pos_inf = false;
  bool has_neg_inf = false;
  size_t num = 0;
  TYPE min = std::numeric_limits<TYPE>::max();
  TYPE max = std::numeric_limits<TYPE>::min();

  // Estimate precision truncating 'precision' digits to the right of the
  // decimal point, and comparing the result to 'tolerance'.  
  // FIXME: Might not be correct for long double.
  int precision = 0;
  TYPE precision_scale = 1;
  TYPE tolerance = fixfmt::pow10(-max_precision) / 2;

  // Note: Weird.  LLVM 6.1.0 on OSX vectorizes this loop only if the precision
  // logic is _present_, with the result that it runs _faster_ than without.

  for (size_t i = 0; i < length; ++i) {
    TYPE const val = array[i];
    // Flag NaN.
    if (std::isnan(val)) {
      has_nan = true;
      continue;
    }
    // Flag positive and negative infinity.
    if (std::isinf(val)) {
      if (val > 0)
        has_pos_inf = true;
      else
        has_neg_inf = true;
      continue;
    }
    // Keep count of non-NaN/infinity values.
    ++num;
    // Compute min and max, excluding NaN and infinity.
    if (val < min)
      min = val;
    if (val > max)
      max = val;
    // Expand precision if necessary to accommodate additional fractional 
    // digits, up to the maximum specified precision.
    while (precision < max_precision) {
      TYPE const scaled = std::abs(val) * precision_scale;
      TYPE const remainder = scaled - (long) (scaled + tolerance);
      if (remainder < tolerance) 
        break;
      else {
        ++precision;
        precision_scale *= 10;
        tolerance *= 10;
      }
    }
  }

  return (ref<Tuple>) (Tuple::builder
    << Bool::from(has_nan)
    << Bool::from(has_pos_inf)
    << Bool::from(has_neg_inf)
    << Long::FromLong(num)
    << Float::FromDouble(min)
    << Float::FromDouble(max)
    << Long::FromLong(precision)
  );
}


ref<Object> center(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {
    "string", "length", "pad", nullptr};
  char const* str;
  int length;
  char const* pad = " ";
  Arg::ParseTupleAndKeywords(
      args, kw_args, "sI|s", arg_names, &str, &length, &pad);

  // FIXME: Validate args.
  if (strlen(pad) == 0)
    throw ValueError("empty pad");

  return Unicode::from(fixfmt::center(string(str), length, pad));
}


ref<Object> pad(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {
    "string", "length", "pad", "position", nullptr};
  char const* str;
  int length;
  char const* pad = " ";
  float position = fixfmt::PAD_POSITION_LEFT_JUSTIFY;
  Arg::ParseTupleAndKeywords(
      args, kw_args, "sI|sf", arg_names, 
      &str, &length, &pad, &position, nullptr);

  // FIXME: Validate args.
  if (strlen(pad) == 0)
    throw ValueError("empty pad");
  if (position < 0 or position > 1)
    throw ValueError("position out of range");

  return Unicode::from(fixfmt::pad(string(str), length, pad, (float) position));
}


ref<Object> elide(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {
    "string", "length", "ellipsis", "position", nullptr};
  char const* str;
  int length;
  char const* ellipsis = fixfmt::ELLIPSIS;
  float position = 1;
  Arg::ParseTupleAndKeywords(
    args, kw_args, "sI|sf", arg_names, &str, &length, &ellipsis, &position);

  string r = fixfmt::elide(string(str), length, string(ellipsis), position);
  return Unicode::from(r);
}


ref<Object> palide(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {
    "string", "length", "ellipsis", "pad", "elide_position", 
    "pad_position", nullptr };
  char const* str;
  int length;
  char const* ellipsis = fixfmt::ELLIPSIS;
  char const* pad = " ";
  float elide_position = 1;
  float pad_position = fixfmt::PAD_POSITION_LEFT_JUSTIFY;
  Arg::ParseTupleAndKeywords(
    args, kw_args, "sI|ssff", arg_names,
    &str, &length, &ellipsis, &pad, &elide_position, &pad_position);

  // FIXME: Validate args.
  if (strlen(pad) == 0)
    throw ValueError("empty pad");

  string r = fixfmt::palide(
    string(str), length, string(ellipsis), pad, elide_position, pad_position);
  return Unicode::from(r);
}


ref<Object> string_length(Module* module, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = { "string", nullptr };
  char* str;

#if PY3K
  Arg::ParseTupleAndKeywords(args, kw_args, "s", arg_names, &str);

  return Long::FromLong(fixfmt::string_length(str));
#else
  Arg::ParseTupleAndKeywords(args, kw_args, "et", arg_names, "utf-8", &str);

  auto const length = fixfmt::string_length(str);
  PyMem_Free(str);
  return Long::FromLong(length);
#endif
}


}  // anonymous namespace

//------------------------------------------------------------------------------

Methods<Module>& add_functions(Methods<Module>& methods)
{
  methods
    .add<analyze_float<double>> ("analyze_double")
    .add<analyze_float<float>>  ("analyze_float")
    .add<center>                ("center")
    .add<elide>                 ("elide")
    .add<pad>                   ("pad")
    .add<palide>                ("palide")
    .add<string_length>         ("string_length")
    ;
  return methods;
}


