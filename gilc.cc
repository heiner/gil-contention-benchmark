
#include <cmath>
#include <pybind11/pybind11.h>

namespace py = pybind11;

constexpr double square(double value) { return value * value; }

class Timing {
public:
  void time() {
    if (++timing_count_ == 1) {
      last_time_ = std::chrono::system_clock::now();
      return;
    };

    if (timing_count_ % 1000 == 0) {
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> delta = now - last_time_;
      last_time_ = std::move(now);
      call(1000.0 / delta.count());
    }
  }

  void call(double value) {
    /* Online update.
       See
         http://www.incompleteideas.net/book/first/ebook/node19.html
       and
         https://math.stackexchange.com/a/103025/5051
    */
    double mean = mean_ + (value - mean_) / (count_ + 1);
    double var =
        (count_ * var_ + count_ * square(mean_ - mean) + square(value - mean)) /
        (count_ + 1);

    ++count_;
    mean_ = mean;
    var_ = var;
  }

  int64_t count() { return count_; }
  double mean() { return mean_; }
  double var() { return var_; }
  double std() { return std::sqrt(var_); }

private:
  int64_t count_ = 0;
  double mean_ = 0.0;
  double var_ = 0.0;

  int64_t timing_count_ = 0;
  std::chrono::time_point<std::chrono::system_clock> last_time_;
};

struct PyTiming {
  PyObject_HEAD /**/;
  Timing *timing;
  static PyObject *time(PyTiming *self) {
    self->timing->time();
    Py_RETURN_NONE;
  }

  static PyObject *time_nogil(PyTiming *self) {
    // clang-format off
      Py_BEGIN_ALLOW_THREADS
      self->timing->time();
      Py_END_ALLOW_THREADS
      Py_RETURN_NONE;
    // clang-format on
  }

  static PyObject *call(PyTiming *self, PyObject *value) {
    double v = PyFloat_AsDouble(value);
    if (PyErr_Occurred())
      return NULL;

    self->timing->call(v);
    Py_RETURN_NONE;
  }

  static PyObject *count(PyTiming *self) {
    return PyLong_FromLong(self->timing->count());
  }

  static PyObject *mean(PyTiming *self) {
    return PyFloat_FromDouble(self->timing->mean());
  }

  static PyObject *var(PyTiming *self) {
    return PyFloat_FromDouble(self->timing->var());
  }

  static PyObject *std(PyTiming *self) {
    return PyFloat_FromDouble(self->timing->std());
  }

  static void tp_dealloc(PyObject *pself) {
    PyTiming *self = (PyTiming *)pself;
    delete self->timing;
    Py_TYPE(self)->tp_free(pself);
  }
  static PyObject *tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyTiming *self = (PyTiming *)type->tp_alloc(type, 0);
    self->timing = new Timing;
    return (PyObject *)self;
  }
  static int tp_init(PyObject *pself, PyObject *args, PyObject *kwds) {
    PyTiming *self = (PyTiming *)pself;
    /* Could set members of self. */
    return 0;
  }
};

static PyTypeObject PyTimingType = {PyVarObject_HEAD_INIT(nullptr, 0)};

static PyMethodDef PyTimingMethods[] = {
    {"time", (PyCFunction)&PyTiming::time, METH_NOARGS, "Doc for time"},
    {"time_nogil", (PyCFunction)&PyTiming::time_nogil, METH_NOARGS, ""},
    {"call", (PyCFunction)&PyTiming::call, METH_O, ""},
    {"count", (PyCFunction)&PyTiming::count, METH_NOARGS, ""},
    {"mean", (PyCFunction)&PyTiming::mean, METH_NOARGS, ""},
    {"var", (PyCFunction)&PyTiming::var, METH_NOARGS, ""},
    {"std", (PyCFunction)&PyTiming::std, METH_NOARGS, ""},
    {nullptr, nullptr, 0, nullptr}};

static PyTypeObject *createPyTimingType() {
  PyTimingType.tp_name = "PyTiming";
  PyTimingType.tp_basicsize = sizeof(PyTiming);
  PyTimingType.tp_itemsize = 0;
  PyTimingType.tp_flags = Py_TPFLAGS_DEFAULT;
  PyTimingType.tp_methods = PyTimingMethods;
  PyTimingType.tp_dealloc = PyTiming::tp_dealloc;
  PyTimingType.tp_init = PyTiming::tp_init;
  PyTimingType.tp_new = PyTiming::tp_new;

  if (PyType_Ready(&PyTimingType) < 0) {
    throw std::runtime_error("PyType_Ready failed");
  }
  Py_INCREF(&PyTimingType);
  return &PyTimingType;
}

PYBIND11_MODULE(gilc, m) {
  py::class_<Timing>(m, "Timing")
      .def(py::init<>())
      .def("time", &Timing::time)
      .def("time_nogil", &Timing::time,
           py::call_guard<py::gil_scoped_release>())
      .def("call", &Timing::call)
      .def("mean", &Timing::mean)
      .def("var", &Timing::var)
      .def("std", &Timing::std)
      .def("count", &Timing::count);

  PyModule_AddObject(m.ptr(), "CTiming", (PyObject *)createPyTimingType());
}
