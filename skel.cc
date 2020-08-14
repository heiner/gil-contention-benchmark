
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Skel {
 public:
  Skel(int64_t number) : number_(number) {}
  int64_t skel() const { return number_; }

 private:
  int64_t number_;
};

PYBIND11_MODULE(skel, m) {
  py::class_<Skel>(m, "Skel")
      .def(py::init<int64_t>(), py::arg("number") = 42)
      .def("skel", &Skel::skel);
}
