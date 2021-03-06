#include <iostream>

#include <Python.h>

#include "PyNumber.hh"
#include "PyTickTime.hh"
#include "fixfmt.hh"
#include "py.hh"

using namespace py;
using std::string;
using std::make_unique;

//------------------------------------------------------------------------------

namespace {

int
get_precision(
  Object* arg)
{
  int precision;
  if (arg == Py_None)
    precision = fixfmt::TickTime::PRECISION_NONE;
  else {
    precision = arg->long_value();
    if (precision < 0)
      precision = fixfmt::TickTime::PRECISION_NONE;
  }
  return precision;
}


int tp_init(PyTickTime* self, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = {
    "scale", "precision", "nat", nullptr };
  long          scale           = fixfmt::TickTime::SCALE_SEC;
  Object*       precision_arg   = (Object*) Py_None;
  char const*   nat             = "NaT";
  Arg::ParseTupleAndKeywords(
    args, kw_args, "|lO$et", arg_names,
    &scale, &precision_arg, "utf-8", &nat);

  if (scale <= 0) 
    throw ValueError("nonpositive scale");
  auto const precision = get_precision(precision_arg);

  new(self) PyTickTime(make_unique<fixfmt::TickTime>(scale, precision, nat));
  return 0;
}


ref<Unicode> tp_repr(PyTickTime* self)
{
  auto const& fmt = self->fmt_;
  std::stringstream ss;
  ss << "TickTime(" << fmt->get_scale() << ", " << fmt->get_precision()
     << ", \"" << fmt->get_nat() << "\")";
  return Unicode::from(ss.str());
}


ref<Object> tp_call(PyTickTime* self, Tuple* args, Dict* kw_args)
{
  static char const* arg_names[] = { "value", nullptr };
  long val;
  Arg::ParseTupleAndKeywords(args, kw_args, "l", arg_names, &val);

  return Unicode::from((*(self->fmt_))(val));
}


auto methods = Methods<PyTickTime>();


ref<Object> get_precision(PyTickTime* const self, void* /* closure */)
{
  return Long::FromLong(self->fmt_->get_precision());
}


ref<Object> get_scale(PyTickTime* const self, void* /* closure */)
{
  return Long::FromLong(self->fmt_->get_scale());
}


ref<Object> get_width(PyTickTime* const self, void* /* closure */)
{
  return Long::FromLong(self->fmt_->get_width());
}


ref<Object> get_nat(PyTickTime* const self, void* /* closure */)
{
  return Unicode::from(self->fmt_->get_nat());
}


auto getsets = GetSets<PyTickTime>()
  .add_get<get_precision>   ("precision")
  .add_get<get_scale>       ("scale")
  .add_get<get_width>       ("width")
  .add_get<get_nat>         ("nat")
  ;


}  // anonymous namespace


Type PyTickTime::type_ = PyTypeObject{
  PyVarObject_HEAD_INIT(nullptr, 0)
  (char const*)         "fixfmt._ext.TickTime",             // tp_name
  (Py_ssize_t)          sizeof(PyTickTime),                 // tp_basicsize
  (Py_ssize_t)          0,                                  // tp_itemsize
  (destructor)          nullptr,                            // tp_dealloc
  (printfunc)           nullptr,                            // tp_print
  (getattrfunc)         nullptr,                            // tp_getattr
  (setattrfunc)         nullptr,                            // tp_setattr
  (PyAsyncMethods*)     nullptr,                            // tp_as_async
  (reprfunc)            wrap<PyTickTime, tp_repr>,          // tp_repr
  (PyNumberMethods*)    nullptr,                            // tp_as_number
  (PySequenceMethods*)  nullptr,                            // tp_as_sequence
  (PyMappingMethods*)   nullptr,                            // tp_as_mapping
  (hashfunc)            nullptr,                            // tp_hash
  (ternaryfunc)         wrap<PyTickTime, tp_call>,          // tp_call
  (reprfunc)            nullptr,                            // tp_str
  (getattrofunc)        nullptr,                            // tp_getattro
  (setattrofunc)        nullptr,                            // tp_setattro
  (PyBufferProcs*)      nullptr,                            // tp_as_buffer
  (unsigned long)       Py_TPFLAGS_DEFAULT
                        | Py_TPFLAGS_BASETYPE,              // tp_flags
  (char const*)         nullptr,                            // tp_doc
  (traverseproc)        nullptr,                            // tp_traverse
  (inquiry)             nullptr,                            // tp_clear
  (richcmpfunc)         nullptr,                            // tp_richcompare
  (Py_ssize_t)          0,                                  // tp_weaklistoffset
  (getiterfunc)         nullptr,                            // tp_iter
  (iternextfunc)        nullptr,                            // tp_iternext
  (PyMethodDef*)        methods,                            // tp_methods
  (PyMemberDef*)        nullptr,                            // tp_members
  (PyGetSetDef*)        getsets,                            // tp_getset
  (_typeobject*)        nullptr,                            // tp_base
  (PyObject*)           nullptr,                            // tp_dict
  (descrgetfunc)        nullptr,                            // tp_descr_get
  (descrsetfunc)        nullptr,                            // tp_descr_set
  (Py_ssize_t)          0,                                  // tp_dictoffset
  (initproc)            tp_init,                            // tp_init
  (allocfunc)           nullptr,                            // tp_alloc
  (newfunc)             PyType_GenericNew,                  // tp_new
  (freefunc)            nullptr,                            // tp_free
  (inquiry)             nullptr,                            // tp_is_gc
  (PyObject*)           nullptr,                            // tp_bases
  (PyObject*)           nullptr,                            // tp_mro
  (PyObject*)           nullptr,                            // tp_cache
  (PyObject*)           nullptr,                            // tp_subclasses
  (PyObject*)           nullptr,                            // tp_weaklist
  (destructor)          nullptr,                            // tp_del
  (unsigned int)        0,                                  // tp_version_tag
  (destructor)          nullptr,                            // tp_finalize
};


